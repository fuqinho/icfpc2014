main:
LD 0 0
LDF get-next-direction
CONS
RTN

(define (nth n lst)
  (if (eq? n 0) (car lst) (nth (n - 1) (cdr lst))))

=nth:
LD 0 0
LDC 0
CEQ
TSEL nth-1 nth-2
==nth-1:
LD 0 1
CAR
RTN
==nth-2:
LD 0 1
CDR
LD 0 0
LDC 1
SUB
DUM 2
LDF nth
TRAP

(define (pos-eq? p1 p2)
  (if (eq? (car p1) (car p2)) (eq? (cdr p1) (cdr p2)) 0))

=pos-eq?:
LD 0 0
CAR
LD 0 1
CAR
CEQ
TSEL pos-eq?-1 pos-eq?-2
==pos-eq?-1:
LD 0 0
CDR
LD 0 1
CDR
CEQ
RTN
==pos-eq?-2:
LDC 0
RTN


(define (find k assoc-lst)
  (if (nil? assoc-lst) 0
      (if (eq? k (car (car assoc-lst)))
          (cdr (car assoc-lst))
          (find k (cdr assoc-lst)))))

=find:
LD 0 1
ATOM
TSEL find-1 find-2
==find-1:
LDC 0
RTN
==find-2:
LD 0 0
LD 0 1
CAR
CAR
LDF pos-eq?
AP
TSEL find-2-1 find-2-2
==find-2-1:
LD 0 1
CAR
CDR
RTN
==find-2-2:
LD 0 1
LD 0 1
CDR
DUM 2
LDF find
TRAP


(define (lst-rev-internal l1 l2)
  (if (nil? l1) l2 (lst-rev-internal (cdr l1) (cons (car l1) l2))))

=lst-rev-internal:
LD 0 0
ATOM
TSEL lst-rev-internal-1 lst-rev-internal-2
==lst-rev-internal-1
LD 0 1
RTN
==lst-rev-internal-2
LD 0 0
CDR
LD 0 0
CAR
LD 0 1
CONS
DUM 2
LDF lst-rev-internal
TRAP

(define (lst-rev lst) (lst-rev-internal lst 0))
=lst-rev
LD 0 0
LDC 0
DUM 2
LDF lst-rev-internal
TRAP

(define q-empty (cons 0 0))
;; N/A

(define (q-push e q) (cons (cons e (car q)) (cdr q)))

=q-push:
LD 0 0
LD 0 1
CAR
CONS
LD 0 1
CDR
CONS
RTN

(define (q-empty? q)
  (and (nil? (car q)) (nil? (cdr q))))
;; N/A

(define (q-take q)
  (if (nil? (cdr q))
      (q-take (cons 0 (lst-rev (car q))))
      (cons (car (cdr q)) (cons (car q) (cdr (cdr q))))))
;;
=q-take
LD 0 0
CDR
ATOM
TSEL q-take-1 q-take-2
==q-take-1
LDC 0
LD 0 0
CAR
LDF lst-rev
AP
CONS
DUM 1
LDF q-take
TRAP
==q-take-2
LD 0 0
CDR
CAR
LD 0 0
CAR
LD 0 0
CDR
CDR
CONS
CONS
RTN

(define (lambda-pos state)
    (car (cdr (car (cdr state)))))

=lambda-pos
LD 0 0
CDR
CAR
CDR
CAR
RTN

(define (get-tile state pos)
  (nth (car pos) (nth (cdr pos) (car state))))
=get-tile
LD 0 1
CAR
LD 0 1
CDR
LD 0 0
CAR
LDF nth
AP
LDF nth
AP
RTN

(define (pos-add pos dir)
  if (eq? dir 0): (cons (car pos) (- (cdr pos) 1))
  if (eq? dir 1): (cons (+ (car pos) 1) (cdr pos))
  if (eq? dir 2): (cons (car pos) (+ (cdr pos) 1))
  (cons (- (car pos) 1) (cdr pos))

==pos-add
LD 0 1
LDC 0
CEQ
TSEL pos-add-up pos-add-1
== pos-add-1
LD 0 1
LDC 1
CEQ
TSEL pos-add-right pos-add-2
== pos-add-2
LD 0 1
LDC 2
CEQ
TSEL pos-add-down pos-add-left

== pos-add-up
LD 0 0
CAR
LD 0 0
CDR
LDC 1
SUB
CONS
RTN

== pos-add-right
LD 0 0
CAR
LDC 1
ADD
LD 0 0
CDR
CONS
RTN

== pos-add-down
LD 0 0
CAR
LD 0 0
CDR
LDC 1
ADD
CONS
RTN

== pos-add-left
LD 0 0
CAR
LDC 1
SUB
LD 0 0
CDR
CONS
RTN

(pos-sub)
== pos-sub
LD 0 1
LDC 0
CEQ
TSEL pos-add-down pos-sub-1
== pos-sub-1
LD 0 1
LDC 1
CEQ
TSEL pos-add-left pos-sub-2
== pos-sub-2
LD 0 1
LDC 2
CEQ
TSEL pos-add-up pos-add-right


(define (get-next-direction-1 state oritinal-pos q assoc-lst)
  let rec iter pos dist tmp =
    tmp := q-take q
    q := cdr tmp
    pos := car tmp
    dist := car (find pos assoc-lst)
    tmp := iter2 pos UP 0
    if tmp: tmp
    tmp := iter2 pos RIGHT 0
    if tmp: tmp
    tmp := iter2 pos DOWN 0
    if tmp: tmp
    tmp := iter2 pos LEFT 0
    if tmp: tmp
    iter 0

==get-next-direction-1-iter
LD 1 2  ;; tmp := q-take q
LDF q-take
AP
ST 0 2
LD 0 2  ;; q <- cdr tmp
CDR
ST 1 2
LD 0 2  ;; pos <- car tmp
CAR
ST 0 0
LD 0 1  ;; dist <- car (find pos assoc-lst)
LD 1 3
LDF find
AP
CAR
ST 0 1

LD 0 0  ;; tmp <- iter2 pos UP 0
LDC 0
LDC 0
LDF get-next-direction-1-iter2
AP
ST 0 2
LD 0 2  ;; if tmp: tmp
ATOM
TSEL get-next-direction-1-iter-2 get-next-direction-1-iter-1
==get-next-direction-1-iter-1
LD 0 2
RTN

==get-next-direction-1-iter-2
LD 0 0  ;; tmp <- iter2 pos RIGHT 0
LDC 1
LDC 0
LDF get-next-direction-1-iter2
AP
ST 0 2
LD 0 2  ;; if tmp: tmp
ATOM
TSEL get-next-direction-1-iter-3 get-next-direction-1-iter-1

==get-next-direction-1-iter-3
LD 0 0  ;; tmp <- iter2 pos DOWN 0
LDC 2
LDC 0
LDF get-next-direction-1-iter2
AP
ST 0 2
LD 0 2  ;; if tmp: tmp
ATOM
TSEL get-next-direction-1-iter-4 get-next-direction-1-iter-1

==get-next-direction-1-iter-5
LD 0 0  ;; tmp <- iter2 pos LEFT 0
LDC 3
LDC 0
LDF get-next-direction-1-iter2
AP
ST 0 2
LD 0 2  ;; if tmp: tmp
ATOM
TSEL get-next-direction-1-iter-5 get-next-direction-1-iter-1

==get-next-direction-1-iter-5
LDC 0
LDC 0
LDC 0
DUM 3
LDF get-next-direction-1-iter
TRAP

  and iter2 pos dir tile =
    pos := pos-add pos dir
    tile := get-tile state pos
    if tile == '#': 0
    if find pos assoc-lst: 0
    assoc-lst := (cons (cons pos (cons (dist + 1) dir)) assoc-lst)
    q := q-push pos q
    if tile == '.' then pos
    if tile == 'o' then pos
    0
  in

==get-next-direction-1-iter2
LD 0 0  ;; pos <- pos-add pos dir
LD 0 1
LDF pos-add
AP
ST 0 0
LD 2 0  ;; tile <- get-tile state pos
LD 0 0
LDF get-tile
AP
ST 0 2
LD 0 2
LDC 0
CEQ
TSEL get-next-direction-1-iter2-notfound get-next-direction-1-iter2-1
==get-next-direction-1-iter2-1
LD 0 0
LD 2 3
LDF find
AP 2
ATOM
TSEL get-next-direction-1-iter2-2 get-next-direction-1-iter2-notfound

==get-next-direction-1-iter2-2
LD 0 0  ;; assoc-lst := (cons (cons pos (cons (dist + 1) dir)) assoc-lst)
LD 1 1
LDC 1
ADD
LD 0 1
CONS
CONS
LD 2 3
CONS
ST 2 3

LD 0 0 ;; q := q-push pos q
LD 2 2
LDF q-push
AP
ST 2 2

LD 0 2
LDC 2
CEQ
TSEL get-next-direction-1-iter2-found get-next-direction-1-iter2-3

==get-next-direction-1-iter2-3
LD 0 2
LDC 3
CEQ
TSEL get-next-direction-1-iter2-found get-next-direction-1-iter2-notfound

==get-next-direction-1-iter2-found
LD 0 0
RTN

==get-next-direction-1-iter2-notfound
LDC 0
RTN

  let rec iter3 pos dir prev-pos =
    dir := cdr (find pos assoc-lst)
    prev-pos := (pos-sub pos dir)
    if prev-pos == original-pos: dir
    iter3 prev-pos 0 0

==get-next-direction-1-iter3
LD 0 0
LD 1 3
LDF find
AP
CDR
ST 0 1
LD 0 0
LD 0 1
LDF pos-sub
AP
ST 0 2
LD 0 2
LD 1 1
LDF pos-eq?
TSEL get-next-direction-1-iter3-1 get-next0direction-1-iter3-2
== get-next-direction-1-iter3-1
LD 0 1
RTN
== get-next-direction-1-iter3-2
LD 0 2
LDC 0
LDC 0
DUM 3
LDF get-next-direction-1-iter3
TRAP


  in
  q := q-push original-pos q
  assoc-lst := (cons (cons original-pos (cons 0 -1)) assoc-lst)
  iter3 (iter 0 0 0) 0 0

=get-next-direction-1
LDC 0
LDC 0
LDC 0
LDF get-next-direction-1-iter
AP 3
LDC 0
LDC 0
LDF get-next-direction-1-iter3
TAP


==get-next-direction
LD 0 1
LD 0 1
LDF lambda-pos
AP 1
LDC 0
LDC 0
CONS
LDC 0
LDF get-next-direction-1
TAP


(define (get-next-direction ai state)
        (get-next-direction-1 state (lambda-pos state)
                              (cons 0 0)  ;; q-empty
                              0  ;; nil assoc-lst



(define (get-next-direction state)

  let q = q-empty in
  let original-pos = lambda-pos state in
  let assoc-lst = [] in
  let assoc-lst = (pos, (0, -1))::assoc-lst in
  let q = q-push q pos in

  let target, assoc-lst =
    let rec iter q assoc-lst =
      let pos, q = q-take q in
      let dist = car (lst-find pos assoc-lst) in

      let q, assoc-lst, result = iter2 (pos + UP) q assoc-lst dist UP in
      if result then result, assoc-lst
      else
        let q, assoc-lst, result = iter2 (pos + RIGHT) q assoc-lst dist RIGHT in
        if result then result, assoc-lst
        else
          let q, assoc-lst, result = iter2 (pos + DOWN) q assoc-lst dist DOWN in
          if result then result, assoc-lst
          else
            let q, assoc-lst, result = iter2 (pos + LEFT) q assoc-lst dist LEFT in
            if result then result, assoc-lst
            else
              iter q assoc-lst
    and iter2 pos q assoc-lst dist dir
      let tile = get-tile state pos in
      if tile == '#' then q, assoc-lst, -1
      else if find-lst pos assoc-lst then q, assoc-lst, -1
      else
        let assoc-lst = (pos, (dist+1, dir))::assoc-lst in
        let q = q-push pos q in
        if tile == '.' then q, assoc-lst, pos
        else if tile == 'o' then q, assoc-lst, pos
        else q, assoc-lst, -1
    in iter q assoc-lst
  in
  let rec iter3 assoc-lst pos =
    let dir = cdr (find pos assoc-lst) in
    let prev = pos + dir in
    if prev == original-pos then dir
    else iter3 assoc-lst prev
  in
  iter3 assoc-lst target
