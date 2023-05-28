; inputError
(set-logic QF_LIRA)
(declare-fun a () Bool)
(declare-fun b () Real)
(assert (= a b))
(check-sat)
