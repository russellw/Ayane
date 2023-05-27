; unsat
(set-logic QF_LIA)
(declare-fun a () Int)
(assert (=  a 5))
(assert
	(let ((x a))
		(=  x 6)
	)
)
(check-sat)
