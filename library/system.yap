/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		system.yap						 *
* Last rev:								 *
* mods:									 *
* comments:	Operating System Access built-ins			 *
*									 *
*************************************************************************/

:- module(system, [
	chmod/2,
	datime/1,
	delete_file/1,
	delete_file/2,
	directory_files/2,
	environ/2,
	exec/3,
	file_exists/1,
	file_exists/2,
	file_property/2,
	fmode/2,
	host_id/1,
	host_name/1,
	pid/1,
	kill/2,
	mktemp/2,
	make_directory/1,
	popen/3,
	rename_file/2,
	shell/1,
	shell/2,
	sleep/1,
	system/1,
	system/2,
	time/1,
	tmpnam/1,
	wait/2,
	working_directory/2
          ]).

:- load_foreign_files([sys], [], init_sys).

% time builtins

datime(X) :-
	datime(X, Error),
	handle_system_error(Error, off, datime(X)).
 
% file operations

delete_file(File) :-
	delete_file(File, off, on, off).

delete_file(File, Opts) :-
	process_delete_file_opts(Opts, Dir, Recurse, Ignore, delete_file(File,Opts)),
	delete_file(File, Dir, Recurse, Ignore).

process_delete_file_opts(V, _, _, _, T) :- var(V), !,
	throw(error(instantiation_error,T)).
process_delete_file_opts([], off, off, off, _) :- !.
process_delete_file_opts([V|_], _, _, _, T) :- var(V), !,
	throw(error(instantiation_error,T)).
process_delete_file_opts([directory|Opts], on, Recurse, Ignore, T) :- !,
	process_delete_file_opts(Opts, _, Recurse, Ignore, T).
process_delete_file_opts([recursive|Opts], Dir, on, Ignore, T) :- !,
	process_delete_file_opts(Opts, Dir, _, Ignore, T).
process_delete_file_opts([ignore|Opts], Dir, Recurse, on, T) :- !,
	process_delete_file_opts(Opts, Dir, Recurse, _, T).
process_delete_file_opts(Opts, _, _, _, T) :-
	throw(error(domain_error(delete_file_option,Opts),T)).

delete_file(File, Dir, Recurse, Ignore) :-
	file_property(File, Type, _, _, _Permissions, Ignore),
	delete_file(Type, File, Dir, Recurse, Ignore).

delete_file(N, File, _Dir, _Recurse, Ignore) :- number(N), !, % error.
	handle_system_error(N, Ignore, delete_file(File)).
delete_file(directory, File, Dir, Recurse, Ignore) :-
	delete_directory(Dir, File, Recurse, Ignore).
delete_file(_, File, _Dir, _Recurse, Ignore) :-
	unlink_file(File, Ignore).

unlink_file(File, Ignore) :-
	unlink(File, N),
	handle_system_error(N, Ignore, delete_file(File)).

delete_directory(on, File, _Recurse, Ignore) :-
	rm_directory(File, Ignore).
delete_directory(off, File, Recurse, Ignore) :-
	delete_directory(Recurse, File, Ignore).

rm_directory(File, Ignore) :-
	rmdir(File, Error),
	handle_system_error(Error, Ignore, delete_file(File)).

delete_directory(on, File, Ignore) :-
	directory_files(File, FileList, Ignore),
	dir_separator(D),
	atom_concat(File, D, FileP),
	delete_dirfiles(FileList, FileP, Ignore),
	rmdir(File, Ignore).

delete_dirfiles([], _, _).
delete_dirfiles(['.'|Fs], File, Ignore) :- !,
	delete_dirfiles(Fs, File, Ignore).
delete_dirfiles(['..'|Fs], File, Ignore) :- !,
	delete_dirfiles(Fs, File, Ignore).
delete_dirfiles([F|Fs], File, Ignore) :-
	atom_concat(File,F,TrueF),
	delete_file(TrueF, off, on, Ignore),
	delete_dirfiles(Fs, File, Ignore).

directory_files(File, FileList) :-
	directory_files(File, FileList, off).

directory_files(File, FileList, Ignore) :-
       list_directory(File, FileList, Error),
       handle_system_error(Error, Ignore, directory_files(File, FileList)).

handle_system_error(Error, _Ignore, _G) :- var(Error), !.
handle_system_error(Error, off, G) :-
	error_message(Error, Message),
	throw(error(system_error(Message),G)).

file_property(File, type(Type)) :-
	file_property(File, Type, _Size, _Date).
file_property(File, size(Size)) :-
	file_property(File, _Type, Size, _Date).
file_property(File, mod_time(Date)) :-
	file_property(File, _Type, _Size, Date).

file_property(File, Type, Size, Date) :-
	file_property(File, Type, Size, Date, _Permissions, Error),
	handle_system_error(Error, off, file_property(File)).

file_exists(File) :-
	file_property(File, _Type, _Size, _Date, _Permissions, Error),
	var(Error).

file_exists(File, Permissions) :-
	file_property(File, _Type, _Size, _Date, FPermissions, Error),
	var(Error),
	process_permissions(Permissions, Perms),
	FPermissions /\ Perms =:= Perms.

process_permissions(Number, Number) :- integer(Number).

make_directory(Dir) :-
	var(Dir), !,
	throw(error(instantiation_error,mkdir(Dir))).
make_directory(Dir) :-
	atom(Dir), !,
	mkdir(Dir,Error),
	handle_system_error(Error, off, mkdir(Dir)).
make_directory(Dir) :-
	throw(error(type_error(atom,Dir),make_directory(Dir))).

rename_file(Old, New) :-
	atom(Old), atom(New), !,
	rename_file(Old, New, Error),
	handle_system_error(Error, off, rename_file(Old, New)).
rename_file(X,Y) :- (var(X) ; var(Y)), !,
	throw(error(instantiation_error,rename_file(X,Y))).
rename_file(X,Y) :- atom(X), !,
	throw(error(type_error(atom,Y),rename_file(X,Y))).
rename_file(X,Y) :-
	throw(error(type_error(atom,X),rename_file(X,Y))).

%
% environment manipulation.
%

environ(Na,Val) :- var(Na), !,
	environ_enum(0,I),
	( p_environ(I,S) -> environ_split(S,SNa,SVal) ; !, fail ),
	atom_codes(Na, SNa),
	atom_codes(Val, SVal).
environ(Na,Val) :- atom(Na), !,
	bound_environ(Na, Val).
environ(Na,Val) :-
	throw(type_error(atom,Na),environ(Na,Val)).

bound_environ(Na, Val) :- var(Val), !,
	getenv(Na,Val).
bound_environ(Na, Val) :- atom(Val), !,
	putenv(Na,Val).
bound_environ(Na, Val) :-
	throw(type_error(atom,Val),environ(Na,Val)).

environ_enum(X,X).
environ_enum(X,X1) :-
	Xi is X+1,
	environ_enum(Xi,X1).

environ_split([61|SVal], [], SVal) :- !.
environ_split([C|S],[C|SNa],SVal) :-
	environ_split(S,SNa,SVal).

working_directory(OLD, NEW) :-
	getcwd(OLD),
	cd(NEW).

%
% process execution
%
exec(Command, [StdIn, StdOut, StdErr], PID) :-
	G = exec(Command, [StdIn, StdOut, StdErr], PID),
	check_command(Command, G), 
	process_inp_stream_for_exec(StdIn, In, G),
	process_out_stream_for_exec(StdOut, Out, G),
	process_err_stream_for_exec(StdErr, Err, G),
	( exec_command(Command, In, Out, Err, PID, Error) -> true ; true ),
	close_temp_streams(StdIn, In, StdOut, Out, StdErr, Err),
	handle_system_error(Error, off, G).

process_inp_stream_for_exec(Error, _, G) :- var(Error), !,
	throw(error(instantiation_error,G)).
process_inp_stream_for_exec(null, 0, _) :- !.
process_inp_stream_for_exec(std, '$stream'(0), _) :- !.
process_inp_stream_for_exec(pipe(SOut), SInp, _) :- !,
	open_pipe_streams(SInp, SOut).
process_inp_stream_for_exec(Stream, Stream, _) :-
	stream_property(Stream, input).


process_out_stream_for_exec(Error, _, G) :- var(Error), !,
	throw(error(instantiation_error,G)).
process_out_stream_for_exec(null, 0, _) :- !.
process_out_stream_for_exec(std, '$stream'(1), _) :- !.
process_out_stream_for_exec(pipe(SInp), SOut, _) :- !,
	open_pipe_streams(SInp, SOut).
process_out_stream_for_exec(Stream, Stream, _) :-
	stream_property(Stream, output).

process_err_stream_for_exec(Error, _, G) :- var(Error), !,
	throw(error(instantiation_error,G)).
process_err_stream_for_exec(null, 0, _) :- !.
process_err_stream_for_exec(std, '$stream'(2), _) :- !.
process_err_stream_for_exec(pipe(SInp), SOut, _) :- !,
	open_pipe_streams(SInp, SOut).
process_err_stream_for_exec(Stream, Stream, _) :-
	stream_property(Stream, output).

close_temp_streams(pipe(_), S, _, _, _, _) :- close(S), fail.
close_temp_streams(_, _, pipe(_), S, _, _) :- close(S), fail.
close_temp_streams(_, _, _, _, pipe(_), S) :- close(S), fail.
close_temp_streams(_, _, _, _, _, _).


popen(Command, Mode, Stream) :-
	G = popen(Command, Mode, Stream),
	check_command(Command, G),
	check_mode(Mode, M, G),
	popen(Command, M, Stream, Result),
	handle_system_error(Result, off, G).

check_command(Com, G) :- var(Com), !,
	throw(error(instantiation_error,G)).
check_command(Com, _) :- atom(Com), !.
check_command(Com, G) :-
	throw(type_error(atom,Com),G).

check_mode(Mode, _, G) :- var(Mode), !,
	throw(error(instantiation_error,G)).
check_mode(read, 0, _) :- !.
check_mode(write,1, _) :- !.
check_mode(Mode, G) :-
	throw(domain_error(io_mode,Mode),G).

shell(Command, Status) :-
	G = shell(Command, Status),
	check_command(Command, G),
	do_shell(Status, Error), 
	( var(Error) -> Status = 0 ; Status = Error).

system(Command, Status) :-
	G = system(Command, Status),
	check_command(Command, G),
	do_system(Command, Status).

sleep(Interval) :- var(Interval), !,
	throw(error(instantiation_error,sleep(Interval))).
sleep(Interval) :- number(Interval), !,
	( Interval =< 0 ->
	    throw(error(domain_error(not_less_than_zero,Interval),
			sleep(Interval)))
	;
	    sleep(Interval, _Remainder)
	).
sleep(Interval) :-
	throw(error(type_error(number,Interval),sleep(Interval))).

wait(PID,STATUS) :- var(PID), !,
	throw(error(instantiation_error,wait(PID,STATUS))).
wait(PID,STATUS) :- integer(PID), !,
	wait(PID, STATUS, Error),
	handle_system_error(Error, off, wait(PID,STATUS)).
wait(PID,STATUS) :-
	throw(error(type_error(integer,PID),wait(PID,STATUS))).

%
% host info
%
host_name(X) :-
	host_name(X, Error),
	handle_system_error(Error, off, host_name(X)).

host_id(X) :-
	host_id(X0, Error),
	handle_system_error(Error, off, host_id(X)),
	number_codes(X0, S),
	atom_codes(X, S).

pid(X) :-
	pid(X, Error),
	handle_system_error(Error, off, pid(X)).

kill(X,Y) :-
	integer(X), integer(Y), !,
	kill(X, Y, Error),
	handle_system_error(Error, off, kill(X,Y)).
kill(X,Y) :- (var(X) ; var(Y)), !,
	throw(error(instantiation_error,kill(X,Y))).
kill(X,Y) :- integer(X), !,
	throw(error(type_error(integer,Y),kill(X,Y))).
kill(X,Y) :-
	throw(error(type_error(integer,X),kill(X,Y))).

mktemp(X,Y) :- var(X), !,
	throw(error(instantiation_error,mktemp(X,Y))).
mktemp(X,Y) :-
	atom(X), !,
	mktemp(X, Y, Error),
	handle_system_error(Error, off, mktemp(X,Y)).
mktemp(X,Y) :-
	throw(error(type_error(atom,X),mktemp(X,Y))).

tmpnam(X) :-
	tmpnam(X, Error),
	handle_system_error(Error, off, tmpnam(X)).
