; sat
(set-logic QF_LIA)
(assert
	(let ((x true))
		(=  x true)
	)
)
(check-sat)
