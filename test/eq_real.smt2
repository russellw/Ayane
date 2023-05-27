; sat
(set-logic QF_LIRA)
(declare-fun a () Real)
(declare-fun b () Real)
(assert (=  a b))
(check-sat)
