; unsat
(declare-fun a () Int)
(assert (=  a 5))
(assert (=  a 6))
(check-sat)
