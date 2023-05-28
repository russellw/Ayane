; inputError
(set-logic QF_LIA)
(assert
	(let ((x y))
		true
	)
)
(check-sat)
