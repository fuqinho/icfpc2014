;; RAND
(define (seed) 0)
(define (rand) (set! seed (+ seed 1) seed))

(define (tick) 0)
(defmacro (set-tick! t e) (set! tick t e))

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

;; Returns modulo. Must be a > 0, b > 0.
(define (mod a b) (- a (* (/ a b) b)))

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
        (if (pos-eq? k (car tkv)) (avl-create (cons k v) l r)
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

;; Returns the fruit's vitality
(define (get-fruit state)
  (cdr (cdr (cdr state))))

;; Returns a tile in the map at the |pos|.
(define (get-tile map pos)
  (nth (car pos) (nth (cdr pos) map)))

;; Returns the number of remaining pills.
(define (get-num-pills map)
  (get-num-pills-iter map 0))

(define (get-num-pills-iter map cnt)
  (if (int? map) cnt
    (get-num-pills-iter (cdr map) (get-num-pills-row (car map) cnt))))

(define (get-num-pills-row row cnt)
  (if (int? row)
      cnt
      (get-num-pills-row (cdr row)
                         (if (= (car row) 2) (+ cnt 1) cnt))))

;; Returns last pills position (if remaining pill is only 1) or 0.
(define (get-last-pill map)
  (get-last-pill-iter map 0 0))

(define (get-last-pill-iter rows row-index result)
  (if (int? rows) result
  (let ((col-index (get-last-pill-row (car rows) 0 0)))
  (if (< col-index 0) 0
  (if (= col-index 0)
      (get-last-pill-iter (cdr rows) (+ row-index 1) result)
  (if (not (int? result)) 0
  (get-last-pill-iter
     (cdr rows) (+ row-index 1) (cons col-index row-index))))))))

(define (get-last-pill-row cols col-index result)
  (if (int? cols) result
  (if (= (car cols) 2)
      (if (> result 0) -1  ;; Two (or more pills) are found.
          (get-last-pill-row (cdr cols) (+ col-index 1) col-index))
      (get-last-pill-row (cdr cols) (+ col-index 1) result))))

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
   (avl-add start (cons 0 5) extra-block)))

(define (bfs-iter map threashold cond q avl)
  (if (q-empty? q) 0
  (let ((pos0 q (q-pop q)))
  (let ((nextdist (+ (car (avl-find pos0 avl)) 1)))
  (if (> nextdist threashold) (bfs-iter map threashold cond q avl)
  (let ((dir-seed (rand)))
  (let ((dir0 (mod dir-seed 4))
        (dir1 (mod (+ dir-seed 1) 4))
        (dir2 (mod (+ dir-seed 2) 4))
        (dir3 (mod (+ dir-seed 3) 4)))
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 dir0) dir0 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 dir1) dir1 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 dir2) dir2 nextdist)))
  (if (not (int? pos)) (cons pos avl)
  (let ((pos q avl
         (bfs-iter2 map cond q avl (pos-add pos0 dir3) dir3 nextdist)))
  (if (not (int? pos)) (cons pos avl)
    (bfs-iter map threashold cond q avl))))))))))))))))

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
(define (find-nearest-ghost map start ghost-lst threashold)
  (let ((bfs-result
         (find-nearest-ghost-internal map start ghost-lst threashold)))
  (if (int? bfs-result) 1000000
  (let ((pos avl bfs-result))
  (car (avl-find pos avl))))))

;;
(define (find-nearest-ghost-internal map start ghost-lst threashold)
  (let ((filtered-ghost-lst
         (lst-filter
            (lambda (ghost)
              (<= (pos-distance (car (cdr ghost)) start) threashold))
            ghost-lst)))
  (if (int? filtered-ghost-lst)
    0
    (bfs map start threashold 0
         (lambda (pos _) (is-ghost pos ghost-lst))))))

;; Returns 1 if there is a ghost at |pos|.
(define (is-ghost pos ghost-lst)
  (if (int? ghost-lst) 0
    (let ((ghost tl ghost-lst))
      (if (pos-eq? pos (car (cdr ghost)))
          1
          (is-ghost pos tl)))))

;;
(define (find-nearest-ghost-dir map start ghost-lst threashold)
  (let ((ghost-lst
         (lst-filter (lambda (ghost) (= (car ghost) 1)) ghost-lst)))
  (let ((bfs-result
         (find-nearest-ghost-internal map start ghost-lst threashold)))
  (if (int? bfs-result) 5
  (let ((pos avl bfs-result))
  (car (backtrack pos start avl)))))))

;; Look for the escape direction.
(define (find-safe-direction map start ghost-lst)
  (let ((danger-zone (build-danger-zone map ghost-lst 3))
        (ghost-block (build-ghost-block-2 ghost-lst 0))
        (last-pill
         (if (< tick 50800) (get-last-pill map) 0)))
  ; First, if last-pill exists, try to search avoiding take it.
  (let ((dir (find-safe-direction-last-pill
                map start ghost-block danger-zone last-pill)))
  (if (>= dir 0) dir
  ;; If not found. Try to search without last pill
  (let ((dir (find-safe-direction-internal map start ghost-block danger-zone)))
  (if (>= dir 0) dir
  ;; If not found. Do not check dead-end.
  (let ((dir (find-safe-direction-no-deadend
                map start ghost-block danger-zone)))
  (if (>= dir 0) dir
  0))))))))  ;; Give up to escape >_<

(define (find-safe-direction-last-pill
             map start ghost-block danger-zone last-pill)
  (if (int? last-pill) -1
  (let ((danger-zone (avl-add last-pill (cons 1 -1) danger-zone)))
  (let ((bfs-result
         (bfs map start 10
              (avl-add last-pill (cons 10000000 5) ghost-block)
              (lambda (pos _)
                (and (int? (avl-find pos danger-zone))
                     (not (is-dead-end map danger-zone pos)))))))
  (if (int? bfs-result) -1
  (car (backtrack (car bfs-result) start (cdr bfs-result))))))))

(define (find-safe-direction-internal map start ghost-block danger-zone)
  (let ((bfs-result
         (bfs map start 10 ghost-block
              (lambda (pos _)
                (and (int? (avl-find pos danger-zone))
                     (not (is-dead-end map danger-zone pos)))))))
  (if (int? bfs-result) -1
  (car (backtrack (car bfs-result) start (cdr bfs-result))))))

(define (find-safe-direction-no-deadend map start ghost-block danger-zone)
  (let ((bfs-result
         (bfs map start 10 ghost-block
              (lambda (pos _) (int? (avl-find pos danger-zone))))))
  (if (int? bfs-result) -1
  (car (backtrack (car bfs-result) start (cdr bfs-result))))))

 ;; give-up to escape.

(define (build-danger-zone map ghost-lst steps)
  (build-danger-zone-iter map ghost-lst steps 0))

(define (build-danger-zone-iter map ghost-lst steps result)
  (if (int? ghost-lst) result
  (let ((ghost ghost-lst ghost-lst))
  (build-danger-zone-iter
     map
     ghost-lst
     steps
     (build-danger-zone-ghost
          map (car (cdr ghost)) (cdr (cdr ghost)) steps result)))))

(define (build-danger-zone-ghost map pos dir steps result)
  (if (= (get-tile map pos) 0) result
  (let ((result (avl-add pos (cons 1 steps) result))
        (steps (- steps 1)))
  (if (< steps 0) result
  (let ((result
         (if (= dir 2)
             result
             (build-danger-zone-ghost map (pos-add pos 0) 0 steps result))))
  (let ((result
         (if (= dir 3)
             result
             (build-danger-zone-ghost map (pos-add pos 1) 1 steps result))))
  (let ((result
         (if (= dir 0)
             result
             (build-danger-zone-ghost map (pos-add pos 2) 2 steps result))))
  (let ((result
         (if (= dir 1)
             result
             (build-danger-zone-ghost map (pos-add pos 3) 3 steps result))))
  result))))))))

(define (build-ghost-block ghost-lst result)
  (if (int? ghost-lst) result
      (build-ghost-block
           (cdr ghost-lst)
           (avl-add (car (cdr (car ghost-lst))) (cons 1000000 5) result))))

(define (build-ghost-block-2 ghost-lst result)
  (if (int? ghost-lst) result
      (build-ghost-block-2
           (cdr ghost-lst)
           (build-ghost-block-2-internal (car ghost-lst) result))))

(define (build-ghost-block-2-internal ghost result)
  (let ((pos (car (cdr ghost)))
        (dir (cdr (cdr ghost))))
  (let ((result (avl-add pos (cons 1000000 5) result)))
  (let ((result
         (if (= dir 2)
             result (avl-add (pos-add pos 0) (cons 1000000 5) result))))
  (let ((result
         (if (= dir 3)
             result (avl-add (pos-add pos 1) (cons 1000000 5) result))))
  (let ((result
         (if (= dir 0)
             result (avl-add (pos-add pos 2) (cons 1000000 5) result))))
  (let ((result
         (if (= dir 1)
             result (avl-add (pos-add pos 3) (cons 1000000 5) result))))
  result)))))))

(define (is-dead-end map danger-zone pos)
  (is-dead-end-iter map danger-zone pos 0))

(define (is-dead-end-iter map danger-zone pos visited)
  (if (is-blocked map danger-zone pos) 1
  (if (not (int? (avl-find pos visited))) 1
  (if (is-junction map danger-zone pos) 0
  (let ((visited (avl-add pos (cons 0 0) visited)))
  (if (not (is-dead-end-iter map danger-zone (pos-add pos 0) visited)) 0
  (if (not (is-dead-end-iter map danger-zone (pos-add pos 1) visited)) 0
  (if (not (is-dead-end-iter map danger-zone (pos-add pos 2) visited)) 0
  (if (not (is-dead-end-iter map danger-zone (pos-add pos 3) visited)) 0
  1)))))))))

(define (is-junction map danger-zone pos)
  (>=
   (+ (not (is-blocked map danger-zone (pos-add pos 0)))
   (+ (not (is-blocked map danger-zone (pos-add pos 1)))
   (+ (not (is-blocked map danger-zone (pos-add pos 2)))
      (not (is-blocked map danger-zone (pos-add pos 3))))))
   3))

(define (is-blocked map danger-zone pos)
  (if (= (get-tile map pos) 0) 1
  (if (not (int? (avl-find pos danger-zone))) 1
  0)))

;; Returns the direction toward a wall block.
(define (find-wall-dir map pos)
  (if (= (get-tile map (pos-add pos 0)) 0) 0
  (if (= (get-tile map (pos-add pos 1)) 0) 1
  (if (= (get-tile map (pos-add pos 2)) 0) 2
  (if (= (get-tile map (pos-add pos 3)) 0) 3
  ;; Not found. Retuns random dir.
  (mod tick 4))))))

;; Main
(define (main)
  (set! seed 0 (set-tick! 127 (cons (cons 0 0) step-function))))

;; Step function.
(define (step-function ai_state state)
  (let ((result (get-next-direction ai_state state)))
  (let ((next-pos (pos-add (lambda-pos state) (cdr result))))
  (let ((tile (get-tile (get-map state) next-pos)))
  (set-tick!
     (+ tick
        (if (or (= tile 2)
                (or (= tile 3)
                    (and (= tile 4) (> (get-fruit state) 0))))
            137
            127))
     result)))))

(define (get-next-direction ai_state state)
  (let ((origin (lambda-pos state))
        (map (get-map state))
        (ghost-lst (get-ghost-list state))
        (fruit (get-fruit state)))
  ;; First check if dangerous mode.
  (let ((plan (dangerous-mode state map origin ghost-lst)))
  (if (not (int? plan)) plan
  ;; Second. Check if power mode.
  (let ((plan (power-mode state map origin ghost-lst)))
  (if (not (int? plan)) plan
  ;; Third, look at cache.
  (let ((plan (cache-mode ai_state (> fruit 0))))
  (if (not (int? plan)) plan
  ;; Fourth, Look for fruit.
  (let ((plan (fruit-mode origin map ghost-lst (> fruit 0))))
  (if (not (int? plan)) plan
  ;; Finally, normal search.
  (let ((plan (search-mode map origin (> fruit 0))))
  (if (not (int? plan)) plan
  ;; Gives up. Just random walk.
  (cons (cons (> fruit 0) 0) (mod (rand) 4))))))))))))))


;; Sub AI definition.

(define (dangerous-mode state map origin ghost-lst)
  (if (or (> (lambda-vitality state) 381)  ;; 127 * 3
          (> (find-nearest-ghost map origin ghost-lst 3) 3))
      0  ;; No-plan
  (let ((safe-direction (find-safe-direction map origin ghost-lst)))
  (cons (cons 0 0) safe-direction))))

(define (power-mode state map origin ghost-lst)
  (if (< (lambda-vitality state) 381)  ;; 127 * 3
      0
  ;; Heruistically, search 7 cells.
  (let ((dir (find-nearest-ghost-dir map origin ghost-lst 7)))
  (if (>= dir 4) 0 ;; fail
  (cons (cons 0 0) dir)))))

(define (cache-mode ai_state fruit-state)
  (let ((prev-fruit-state cache ai_state))
  (if (or (not (= fruit-state prev-fruit-state))
          (int? cache)) 0  ;; No cache available.
  (cons (cons fruit-state (cdr cache)) (car cache)))))

(define (fruit-mode origin map ghost-lst fruit-state)
  (let ((last-pill (get-last-pill map)))
  (if (and (not fruit-state)
           (not (and (not (int? last-pill)) (< tick 50800))))
      0 ;; There is more than 1 pills and no-fruit is shown.
  (if (= (get-tile map origin) 4)
      ;; At fruit position. Try to stay there.
      (cons (cons fruit-state 0) (find-wall-dir map origin))
  ;; Search a path to fruit, and move toward there.
  (let ((base-block
         (if (int? last-pill)
             0
             (avl-add last-pill (cons 10000000 5) 0))))
  (let ((bfs-result
         (bfs map origin 100000000
              (build-ghost-block ghost-lst base-block)
              (lambda (_ tile) (= tile 4)))))
  (if (int? bfs-result) 0 ;; Not found
  (let ((trace (backtrack (car bfs-result) origin (cdr bfs-result))))
  (if (int? trace) 0
  (cons (cons fruit-state (cdr trace)) (car trace)))))))))))

(define (search-mode map origin fruit-state)
  (let ((bfs-result
         (bfs map origin 100000000 0
              (lambda (_ tile) (or (= tile 2) (= tile 3))))))
  (if (int? bfs-result) 0
  (let ((trace (backtrack (car bfs-result) origin (cdr bfs-result))))
  (cons (cons fruit-state (cdr trace)) (car trace))))))
