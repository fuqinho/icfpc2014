(define (not a) (= a 0))

(define (nth n lst)
  (if (= n 0) (car lst) (nth (- n 1) (cdr lst))))

(define (pos-eq? p1 p2)
  (if (= (car p1) (car p2)) (= (cdr p1) (cdr p2)) 0))

(define (find key-pos assoc-lst)
  (if (int? assoc-lst)
      0
      (if (pos-eq? key-pos (car (car assoc-lst)))
          (cdr (car assoc-lst))
          (find key-pos (cdr assoc-lst)))))

(define (lst-rev-internal lst1 lst2)
  (if (int? lst1)
      lst2
      (lst-rev-internal (cdr lst1) (cons (car lst1) lst2))))

(define (lst-rev lst) (lst-rev-internal lst 0))

(define (q-push e q) (cons (cons e (car q)) (cdr q)))

(define (q-pop q)
  (if (int? (cdr q))
      (q-pop (cons 0 (lst-rev (car q))))
      (cons (car (cdr q)) (cons (car q) (cdr (cdr q))))))

(define (lambda-pos state)
  (car (cdr (car (cdr state)))))

(define (get-tile state pos)
  (nth (car pos) (nth (cdr pos) (car state))))

(define (pos-add pos dir)
  (if (= dir 0) (cons (car pos) (- (cdr pos) 1))
  (if (= dir 1) (cons (+ (car pos) 1) (cdr pos))
  (if (= dir 2) (cons (car pos) (+ (cdr pos) 1))
                (cons (- (car pos) 1) (cdr pos))))))

(define (pos-sub pos dir)
  (if (= dir 0) (cons (car pos) (+ (cdr pos) 1))
  (if (= dir 1) (cons (- (car pos) 1) (cdr pos))
  (if (= dir 2) (cons (car pos) (- (cdr pos) 1))
                (cons (+ (car pos) 1) (cdr pos))))))

(define (main) (cons 0 get-next-direction))


(define (get-next-direction ai_state state)
  ((lambda (original-pos)
     ((lambda (search-result)
        (cons 0
              (find-next-move
                   (car search-result) original-pos (cdr search-result))))
      (get-next-direction-1
       state
       (q-push original-pos (cons 0 0))
       (cons (cons original-pos (cons 0 5)) 0))))
   (lambda-pos state)))

(define (get-next-direction-1 state q assoc-lst)
  (let ((pop-result (q-pop q)))
    (let ((pos0 (car pop-result))
          (q (cdr pop-result)))
      (let ((nextdist (+ (car (find pos0 assoc-lst)) 1)))
        (get-next-direction-2
         state q assoc-lst (pos-add pos0 0) 0 nextdist
         (lambda (pos q assoc-lst)
           (if (not (int? pos)) (cons pos assoc-lst)
             (get-next-direction-2
              state q assoc-lst (pos-add pos0 1) 1 nextdist
              (lambda (pos q assoc-lst)
                (if (not (int? pos)) (cons pos assoc-lst)
                  (get-next-direction-2
                   state q assoc-lst (pos-add pos0 2) 2 nextdist
                   (lambda (pos q assoc-lst)
                     (if (not (int? pos)) (cons pos assoc-lst)
                       (get-next-direction-2
                        state q assoc-lst (pos-add pos0 3) 3 nextdist
                        (lambda (pos q assoc-lst)
                          (if (not (int? pos)) (cons pos assoc-lst)
                            (get-next-direction-1
                             state q assoc-lst)))))))))))))))))

(define (get-next-direction-2 state q assoc-lst pos dir dist cont)
  (let ((tile (get-tile state pos)))
    (if (= tile 0) (cont 0 q assoc-lst)
    (if (not (int? (find pos assoc-lst))) (cont 0 q assoc-lst)
      (let ((q (q-push pos q))
            (assoc-lst (cons (cons pos (cons dist dir)) assoc-lst)))
        (if (= tile 2) (cont pos q assoc-lst)
        (if (= tile 3) (cont pos q assoc-lst)
          (cont 0 q assoc-lst))))))))

(define (find-next-move pos original-pos assoc-lst)
  (let ((dir (cdr (find pos assoc-lst))))
    (let ((prev-pos (pos-sub pos dir)))
      (if (pos-eq? original-pos prev-pos)
          dir
          (find-next-move prev-pos original-pos assoc-lst)))))
