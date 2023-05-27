; unsat
(declare-fun a () Int)
(assert (=  a 5))
(let ((x a))
	(assert (=  x 6))
)
(check-sat)
