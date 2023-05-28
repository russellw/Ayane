; sat
(set-logic LIA)
(assert
	(exists((x Int))
	(let ((y x))
		(=  y 6)
	)
	)
)
(check-sat)
