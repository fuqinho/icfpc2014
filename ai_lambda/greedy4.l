;; Returns not of boolean value.
(define (not a) (= a 0))

;; Returns n-th element of a list.
(define (nth n lst)
  (if (= n 0) (car lst) (nth (- n 1) (cdr lst))))

;; Compares two positions. Returns true if equals.
(define (pos-eq? p1 p2)
  (if (= (car p1) (car p2)) (= (cdr p1) (cdr p2)) 0))

;; Finds a value from assoc list, whose key is position.
;; If not found '0' is returned.
(define (find key-pos assoc-lst)
  (if (int? assoc-lst)
      0
      (if (pos-eq? key-pos (car (car assoc-lst)))
          (cdr (car assoc-lst))
          (find key-pos (cdr assoc-lst)))))

;; Reverses the list.
(define (lst-rev lst) (lst-rev-internal lst 0))

(define (lst-rev-internal lst1 lst2)
  (if (int? lst1)
      lst2
      (lst-rev-internal (cdr lst1) (cons (car lst1) lst2))))

;; Pushes an element into a queue. Returns a new queue.
(define (q-push e q) (cons (cons e (car q)) (cdr q)))

;; Pops an element from a queue. Returns a pair of element and a new queue.
(define (q-pop q)
  (if (int? (cdr q))
      (q-pop (cons 0 (lst-rev (car q))))
      (cons (car (cdr q)) (cons (car q) (cdr (cdr q))))))

;; Returns the lambda-man's position.
(define (lambda-pos state)
  (car (cdr (car (cdr state)))))

;; Returns the map data.
(define (get-map state) (car state))

;; Returns a tile in the map at the |pos|.
(define (get-tile map pos)
  (nth (car pos) (nth (cdr pos) map)))

;; Returns opposite direction.
(define (opposite dir)
  (if (= dir 0) 2
  (if (= dir 1) 3
  (if (= dir 2) 0
                1))))

;; Returns neighbor position along the direction.
(define (pos-add pos dir)
  (if (= dir 0) (cons (car pos) (- (cdr pos) 1))
  (if (= dir 1) (cons (+ (car pos) 1) (cdr pos))
  (if (= dir 2) (cons (car pos) (+ (cdr pos) 1))
                (cons (- (car pos) 1) (cdr pos))))))

;; Returns neighbot position alog the opposite direction.
(define (pos-sub pos dir) (pos-add pos (opposite dir)))


;; BFS. Takes map, starting position, and condition-function.
;; Condition will take pos and tile.
(define (bfs map start cond)
  (bfs-iter
   map
   cond
   (q-push start (cons 0 0))
   (cons (cons start (cons 0 5)) 0)))

(define (bfs-iter map cond q assoc-lst)
  (let ((pos0 q (q-pop q)))
  (let ((nextdist (+ (car (find pos0 assoc-lst)) 1)))
  (let ((pos q assoc-lst
         (bfs-iter2 map cond q assoc-lst (pos-add pos0 0) 0 nextdist)))
  (if (not (int? pos)) (cons pos assoc-lst)
  (let ((pos q assoc-lst
         (bfs-iter2 map cond q assoc-lst (pos-add pos0 1) 1 nextdist)))
  (if (not (int? pos)) (cons pos assoc-lst)
  (let ((pos q assoc-lst
         (bfs-iter2 map cond q assoc-lst (pos-add pos0 2) 2 nextdist)))
  (if (not (int? pos)) (cons pos assoc-lst)
  (let ((pos q assoc-lst
         (bfs-iter2 map cond q assoc-lst (pos-add pos0 3) 3 nextdist)))
  (if (not (int? pos)) (cons pos assoc-lst)
    (bfs-iter map cond q assoc-lst))))))))))))

(define (bfs-iter2 map cond q assoc-lst pos dir dist)
  (let ((tile (get-tile map pos)))
  (if (= tile 0) (cons 0 q assoc-lst)
  (if (not (int? (find pos assoc-lst))) (cons 0 q assoc-lst)
  (let ((q (q-push pos q))
        (assoc-lst (cons (cons pos (cons dist dir)) assoc-lst)))
  (if (cond pos tile) (cons pos q assoc-lst)
    (cons 0 q assoc-lst)))))))

;; Backtracking. Returns the initial direction from |origin|.
;; Must be |pos| != |origin|.
(define (backtrack pos origin assoc-lst)
  (let ((dir (cdr (find pos assoc-lst))))
  (let ((prev-pos (pos-sub pos dir)))
  (if (pos-eq? origin prev-pos) dir
    (backtrack prev-pos origin assoc-lst)))))

;; Returns the (cons distance direction)
(define (find-nearest-ghost map start ghost-lst)
  (let ((pos assoc-lst
         (bfs map start
              (lambda (pos _) (is-ghost pos ghost-lst)))))
  (cons (car (find pos assoc-lst))
        (backtrack pos start assoc-lst))))

;; Returns 1 if there is a ghost at |pos|.
(define (is-ghost pos ghost-lst)
  (if (int? ghost-lst) 0
    (let ((ghost tl ghost-lst))
      (if (pos-eq? pos (car (cdr ghost)))
          1
          (is-ghost pos tl)))))

;; Main
(define (main) (cons 0 get-next-direction))

;; Step function.
(define (get-next-direction ai_state state)
  (let ((origin (lambda-pos state)))
  (let ((pos assoc-lst
         (bfs (get-map state)
              origin
              (lambda (_ tile) (if (= tile 2) 1 (= tile 3))))))
  (cons 0 (backtrack pos origin assoc-lst)))))
