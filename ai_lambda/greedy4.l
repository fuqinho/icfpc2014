;; Boolean operation with short circuit.
(defmacro (or x y) (if x 1 y))
(defmacro (and x y) (if x y 0))

;; Returns not of boolean value.
(define (not a) (= a 0))

;; Returns bigger one.
(define (max a b) (if (> a b) a b))

;; Returns smaller one.
(define (min a b) (if (< a b) a b))

;; Returns an abs value of the given value.
(define (abs a) (if (< a 0) (- 0 a) a))

;; Position operations.

;; Compares two positions. Returns true if equals.
(define (pos-eq? p1 p2)
  (and (= (car p1) (car p2)) (= (cdr p1) (cdr p2))))

;; Compares two positions. Returns true if p1 < p2 in lexicographical order.
(define (pos-lt? p1 p2)
  (or (< (car p1) (car p2))
      (and (= (car p1) (car p2)) (< (cdr p1) (cdr p2)))))

;; Returns manhattan distance between p1 and p2.
(define (pos-distance p1 p2)
  (+ (abs (- (car p1) (car p2))) (abs (- (cdr p1) (cdr p2)))))

;; List operations.

;; Returns n-th element of a list.
(define (nth n lst)
  (if (= n 0) (car lst) (nth (- n 1) (cdr lst))))

;; Reverses the list.
(define (lst-rev lst) (lst-rev-internal lst 0))
(define (lst-rev-internal lst1 lst2)
  (if (int? lst1)
      lst2
      (lst-rev-internal (cdr lst1) (cons (car lst1) lst2))))

;; Filters the list element by the given function.
(define (lst-filter fun lst)
  (if (int? lst) 0
  (if (fun (car lst))
      (cons (car lst) (lst-filter fun (cdr lst)))
      (lst-filter fun (cdr lst)))))

;; Queue operations.
;; Queue is represented by a pair of lists.
;; Empty queue is (cons 0 0) (meaning (cons nil nil)).

;; Pushes an element into a queue. Returns a new queue.
(define (q-push e q) (cons (cons e (car q)) (cdr q)))

;; Pops an element from a queue. Returns a pair of element and a new queue.
;; Must not be empty.
(define (q-pop q)
  (if (int? (cdr q))
      (q-pop (cons 0 (lst-rev (car q))))
      (cons (car (cdr q)) (cons (car q) (cdr (cdr q))))))

;; Returns true if queue is empty.
(define (q-empty? q)
  (and (int? (car q)) (int? (cdr q))))

;; Map operations. Here Map is implemented by AVL tree.
;; An AVL node is represeneted (((k v) h) (l r)) where
;; k: key (position)
;; v: value
;; h: height
;; l: left child tree, and
;; r: right child tree
;; To represent an empty node, 0 is used.

;; Returns true if the tree is empty.
(define (avl-empty? tree) (int? tree))

;; Returns the height of the tree.
(define (avl-height tree)
  (if (int? tree) 0 (cdr (car tree))))

(define (avl-create kv l r)
  (let ((hl (avl-height l))
        (hr (avl-height r)))
    (cons (cons kv (+ (max hl hr) 1)) (cons l r))))

(define (avl-bal kv l r)
  (let ((hl (avl-height l))
        (hr (avl-height r)))
    (if (> hl (+ hr 2))
        (let ((lkv (car (car l)))
              (ll (car (cdr l)))
              (lr (cdr (cdr l))))
          (if (>= (avl-height ll) (avl-height lr))
              (avl-create lkv ll (avl-create kv lr r))
              (let ((lrkv (car (car lr)))
                    (lrl (car (cdr lr)))
                    (lrr (cdr (cdr lr))))
                (avl-create lrkv (avl-create lkv ll lrl)
                                 (avl-create kv lrr r)))))
    (if (> hr (+ hl 2))
        (let ((rkv (car (car r)))
              (rl (car (cdr r)))
              (rr (cdr (cdr r))))
          (if (>= (avl-height rr) (avl-height rl))
              (avl-create rkv (avl-create kv l rl) rr)
              (let ((rlkv (car (car rl)))
                    (rll (car (cdr rl)))
                    (rlr (cdr (cdr rl))))
                (avl-create rlkv (avl-create kv l rll)
                                 (avl-create rkv rlr rr)))))
    (cons (cons kv (+ (max hl hr) 1)) (cons l r))))))

(define (avl-add k v tree)
  (if (int? tree)
      (cons (cons (cons k v) 1) (cons 0 0))
      (let ((tkv (car (car tree)))
            (l (car (cdr tree)))
            (r (cdr (cdr tree))))
        (if (pos-eq? k (car tkv)) tree
        (if (pos-lt? k (car tkv))
            (avl-bal tkv (avl-add k v l) r)
            (avl-bal tkv l (avl-add k v r)))))))

(define (avl-find k tree)
  (if (int? tree)
      0
      (let ((tkv (car (car tree))))
        (if (pos-eq? k (car tkv)) (cdr tkv)
        (if (pos-lt? k (car tkv))
            (avl-find k (car (cdr tree)))
            (avl-find k (cdr (cdr tree))))))))

;; Finds a value from assoc list, whose key is position.
;; If not found '0' is returned.
(define (find key-pos assoc-lst)
  (if (int? assoc-lst)
      0
      (if (pos-eq? key-pos (car (car assoc-lst)))
          (cdr (car assoc-lst))
          (find key-pos (cdr assoc-lst)))))


;; Returns the lambda-man's position.
(define (lambda-pos state)
  (car (cdr (car (cdr state)))))

;; Returns the lambda-man's vitality.
(define (lambda-vitality state)
  (car (car (cdr state))))

;; Returns the map data.
(define (get-map state) (car state))

;; Reutns a list of ghost.
(define (get-ghost-list state)
  (car (cdr (cdr state))))

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
(define (bfs map start threashold extra-block cond)
  (bfs-iter
   map
   threashold
   cond
   (q-push start (cons 0 0))
   (avl-add start (cons 0 5)
            (if (int? extra-block)
                0
                (avl-add extra-block (cons 0 5) 0)))))

(define (bfs-iter map threashold cond q avl)
  (if (q-empty? q) 0
  (let ((pos0 q (q-pop q)))
  (let ((nextdist (+ (car (avl-find pos0 avl)) 1)))
  (if (> nextdist threashold) (bfs-iter map threashold cond q avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 0) 0 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 1) 1 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 2) 2 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 3) 3 nextdist)))
  (if (not (int? pos)) (cons pos avl)
    (bfs-iter map threashold cond q avl))))))))))))))

(define (bfs-iter2 map cond q avl pos dir dist)
  (let ((tile (get-tile map pos)))
  (if (or (= tile 0) (not (int? (avl-find pos avl)))) (cons 0 q avl)
  (let ((q (q-push pos q))
        (avl (avl-add pos (cons dist dir) avl)))
  (if (cond pos tile) (cons pos q avl)
    (cons 0 q avl))))))

;; Backtracking. Returns the initial direction from |origin|.
;; Must be |pos| != |origin|.
(define (backtrack pos origin avl)
  (backtrack-iter pos origin avl 0))

(define (backtrack-iter pos origin avl result)
  (let ((dir (cdr (avl-find pos avl))))
  (let ((prev-pos (pos-sub pos dir))
        (result (cons dir result)))
  (if (pos-eq? origin prev-pos) result
    (backtrack-iter prev-pos origin avl result)))))



;; Returns the distance.
(define (find-nearest-ghost map start ghost-lst threashold extra-block)
  (let ((filtered-ghost-lst
         (lst-filter
            (lambda (ghost)
              (<= (pos-distance (car (cdr ghost)) start) threashold))
            ghost-lst)))
  (if (int? filtered-ghost-lst) 100000000
  (let ((bfs-result
         (bfs map start threashold extra-block
              (lambda (pos _) (is-ghost pos ghost-lst)))))
  (if (int? bfs-result) 1000000
  (let ((pos avl bfs-result))
  (car (avl-find pos avl))))))))

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
  (let ((origin (lambda-pos state))
        (map (get-map state))
        (ghost-lst (get-ghost-list state)))
  (if (and (< (lambda-vitality state) (* 127 3))
           (<= (find-nearest-ghost map origin ghost-lst 3 0) 3))
      (cons 0 (find-farthest-ghost-direction map origin ghost-lst))
      (if (not (int? ai_state)) (cons (cdr ai_state) (car ai_state))
      (let ((pos avl
                 (bfs map
                      origin
                      1000000000
                      0
                      (lambda (_ tile) (if (= tile 2) 1 (= tile 3))))))
      (let ((trace (backtrack pos origin avl)))
        (cons (cdr trace) (car trace))))))))

(define (find-farthest-ghost-direction map origin ghost-lst)
  (let ((up (pos-add origin 0))
        (right (pos-add origin 1))
        (down (pos-add origin 2))
        (left (pos-add origin 3)))
  (let ((up-distance
         (if (= (get-tile map up) 0)
             0
             (find-nearest-ghost map up ghost-lst 10 origin)))
        (right-distance
         (if (= (get-tile map right) 0)
             0
             (find-nearest-ghost map right ghost-lst 10 origin)))
        (down-distance
         (if (= (get-tile map down) 0)
             0
             (find-nearest-ghost map down ghost-lst 10 origin)))
        (left-distance
         (if (= (get-tile map left) 0)
             0
             (find-nearest-ghost map left ghost-lst 10 origin))))
  (let ((farthest-distance up-distance)
        (farthest-direction 0))
  (let ((farthest-distance farthest-direction
        (if (> right-distance farthest-distance)
            (cons right-distance 1)
            (cons farthest-distance farthest-direction))))
  (let ((farthest-distance farthest-direction
        (if (> down-distance farthest-distance)
            (cons down-distance 2)
            (cons farthest-distance farthest-direction))))
  (let ((farthest-distance farthest-direction
        (if (> left-distance farthest-distance)
            (cons left-distance 3)
            (cons farthest-distance farthest-direction))))
  farthest-direction)))))))
