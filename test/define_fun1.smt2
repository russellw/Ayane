; unsat
(set-logic QF_LIA)
(define-fun f ((x Int)) Int (+ x 1))
(assert (= (f 5) 7))
(check-sat)
