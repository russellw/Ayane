; sat
(set-logic LIA)
(assert
	(exists ((x Int))
		(=  x 5)
	)
)
(check-sat)
