; unsat
(set-logic LIA)
(declare-fun a () Int)
(assert
	(exists ((x Int))
	(let ((x 5))
		(=  x 6)
	)
	)
)
(check-sat)
