; sat
(set-logic QF_LIA)
(assert
	(let ((x 5)(y 5))
		(=  x 5)
	)
)
(check-sat)
