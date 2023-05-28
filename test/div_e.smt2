; sat
(set-logic QF_NIA)
(assert
	(and
		(= (div 7 3) 2)
		(= (div (- 7) 3) (- 3))
		(= (div 7 (- 3)) (- 2))
		(= (div (- 7) (- 3)) 3)
	)
)
(check-sat)
