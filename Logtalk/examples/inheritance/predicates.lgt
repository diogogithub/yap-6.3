
:- category(predicates).


	:- public(public/0).
	:- mode(public, one).

	:- protected(protected/0).
	:- mode(protected, one).

	:- private(private/0).
	:- mode(private, one).


	public :-
		write('Public predicate declared and defined in category predicates.'), nl.

	protected :-
		write('Protected predicate declared and defined in category predicates.'), nl.

	private :-
		write('Private predicate declared and defined in category predicates.'), nl.


:- end_category.
