:- op(600,xfy,::).
:- op(600,fy,::).
:- op(600,fx,^^).
:- dynamic '-2__ddcl'/4.
:- dynamic '-2__ddef'/5.
'-2_'('-2__dcl','-2__def','-2__super','-2__sdcl','-2__sdef','-2__ddcl','-2__ddef').
'-2__dcl'(_17137,_17138,_17139,_17140):-fail.
'-2__dcl'(_17137,_17138,_17139,_17140,_17144-_17145,_17144-_17145):-'-2__dcl'(_17137,_17138,_17139,_17140).
'-2__dcl'(_17137,_17138,_17139,_17140,_17144-_17145,_17144-_17145):-'-2__ddcl'(_17137,_17138,_17139,_17140).
'-2__dcl'(_17137,_17138,_17139,_17140,_17144-_17145,_17142):-symdiffp0__dcl(_17137,_17138,_17139,_17140,_17142).
'-2__def'(diff(_17140),_17135,_17136,_17137,'-2_diff1'(_17140,_17135,_17136,_17137)).
'-2__def'(diff(_17140,_17141,_17142),_17135,_17136,_17137,'-2_diff3'(_17140,_17141,_17142,_17135,_17136,_17137)).
'-2__def'(simplify(_17140),_17135,_17136,_17137,'-2_simplify1'(_17140,_17135,_17136,_17137)).
'-2__def'(simplify(_17140,_17141,_17142),_17135,_17136,_17137,'-2_simplify3'(_17140,_17141,_17142,_17135,_17136,_17137)).
'-2__def'(_17137,_17138,_17139,_17140,_17141,_17144-_17145):-'-2__def'(_17137,_17138,_17139,_17140,_17141).
'-2__def'(_17137,_17138,_17139,_17140,_17141,_17144-_17145):-'-2__ddef'(_17137,_17138,_17139,_17140,_17141).
'-2__super'(_17137,_17138,_17139,_17140,_17141,_17142):-fail.
'-2_diff1'(_17137,_17138,_17142-_17143,_17140):-once('-2_diff3'(_17142,_17143,_17137,_17138,_17142-_17143,_17140)).
'-2_diff3'(_17137,_17138,0,_17140,_17141,_17142):-integer(_17137),integer(_17138).
'-2_diff3'(_17137,_17138,_17139,_17140,_17141,_17142):-integer(_17138),lgt_send_to_object(_17137,diff(_17139),_17141).
'-2_diff3'(_17137,_17138,_17139,_17140,_17141,_17142):-integer(_17137),lgt_send_to_object(_17138,diff(_17139),_17141).
'-2_diff3'(_17137,_17138,_17144-_17145,_17140,_17141,_17142):-lgt_send_to_object(_17137,diff(_17144),_17141),lgt_send_to_object(_17138,diff(_17145),_17141).
'-2_simplify1'(_17137,_17138,_17142-_17143,_17140):-once('-2_simplify3'(_17142,_17143,_17137,_17138,_17142-_17143,_17140)).
'-2_simplify3'(_17134,_17134,0,_17137,_17138,_17139).
'-2_simplify3'(_17137,_17138,_17139,_17140,_17141,_17142):-integer(_17137),integer(_17138),_17139 is _17137-_17138.
'-2_simplify3'(_17137,0,_17139,_17140,_17141,_17142):-lgt_send_to_object(_17137,simplify(_17139),_17141).
'-2_simplify3'(0,_17138,_17139,_17140,_17141,_17142):-lgt_send_to_object(_17138,simplify(_17139),_17141).
'-2_simplify3'(_17137,_17138,_17144-_17138,_17140,_17141,_17142):-integer(_17138),lgt_send_to_object(_17137,simplify(_17144),_17141).
'-2_simplify3'(_17137,_17138,_17137-_17145,_17140,_17141,_17142):-integer(_17137),lgt_send_to_object(_17138,simplify(_17145),_17141).
'-2_simplify3'(_17137,_17138,_17139,_17140,_17141,_17142):-lgt_send_to_object(_17137,simplify(_17151),_17141),lgt_send_to_object(_17138,simplify(_17160),_17141),(_17137-_17138\=_17151-_17160->lgt_send_to_object_nv(_17151-_17160,simplify(_17139),_17141);_17139=_17151-_17160).
:- initialization((lgt_assert_relation_clauses([lgt_current_object_(_17151-_17152,'-2_','-2__dcl','-2__def','-2__super'),lgt_implements_protocol_(_17160-_17161,symdiffp,public)]))).
