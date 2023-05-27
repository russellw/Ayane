; unsat
(set-logic LIA)
(assert
	(forall ((x Int))
		(=  x 5)
	)
)
(check-sat)
