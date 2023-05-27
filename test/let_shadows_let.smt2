; unsat
(set-logic QF_LIA)
(declare-fun a () Int)
(assert (=  a 5))
(assert
	(let ((x 5))
	(let ((x 6))
		(=  x a)
	)
	)
)
(check-sat)
