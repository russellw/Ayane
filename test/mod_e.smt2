; sat
(set-logic QF_NIA)
(assert
	(and
		(= (mod 7 3) 1)
		(= (mod (- 7) 3) 2)
		(= (mod 7 (- 3)) 1)
		(= (mod (- 7) (- 3)) 2)
	)
)
(check-sat)
