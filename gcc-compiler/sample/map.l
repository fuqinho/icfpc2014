(define (decr x)
	(lambda (y) (- y x)))

(define (map f xs)
	(if (int? xs) 0 (cons (f (car xs)) (map f (cdr xs)))))

(define (main)
	(map (decr 3)
		(cons 10
			(cons 20
				(cons 30 0)))))
