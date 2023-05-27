; sat
(set-logic LIA)
(declare-fun a () Int)
(assert
	(let ((x 5))
	(exists ((x Int))
		(=  x 6)
	)
	)
)
(check-sat)
