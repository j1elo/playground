#| DOCUMENTACION GENERAL Y NORMAS DE USO

This file contains a incomplete Lisp implementation of the ID3 program for
learning decision trees, and other functions to generate random trees.  
The seven key functions are:

 1. id3-tree(examples target.attribute attributes) learns a decision tree, 
    using a simplified version of the ID3 algorithm 

 2. random-tree(examples target.attribute attributes) generates a random tree 
 
 3. Classify(instance tree)
    Given a new instance and a previously learned tree, returns the 
    classification of that instance produced by the tree.
 
 4. Print.tree(tree)
    Prints a tree in human readable form
 
 5. Print.entity(instance)
    Prints an instance (e.g., one of your training examples)
 
 6. Get.value(attribute instance)
    Get the value of some ATTRIBUTE of INSTANCE
 
 7. Put.value(attribute instance value)
    Assign VALUE to ATTRIBUTE of INSTANCE

EXAMPLE:
  Loading the code for Decision Tree construction (this file):
  >> (load "id3-trees.lisp")
  >> (load "random-trees.lisp")

  Load training data:
  >> (inicializar.datos "tennis.dat")

  ;; FIRST LOOK AT THE PROVIDED TRAINING EXAMPLES
  >> *training.examples*
  (D14 D13 D12 D11 D10 D9 D8 D7 D6 D5 ...)

  ;; USE THE FUNCTION PRINT.ENTITY TO EXAMINE JUST ONE OF THEM
  >> (print.entity 'd6)

  (PLAY.TENNIS? NO WIND STRONG HUMIDITY NORMAL TEMPERATURE COOL OUTLOOK RAIN) 

  ;; NOW USE THE FUNCTION TREE TO LEARN A TREE
  >> (setq id3-tree (tree *training.examples* 
  		  'play.tennis? 
		  '(outlook temperature humidity wind)))

  (OUTLOOK (SUNNY (HUMIDITY (NORMAL YES) (HIGH NO))) 
           (OVERCAST YES)
           (RAIN (WIND (STRONG NO) (WEAK YES))))

  ;;; OR TRY IT ALSO THIS OTHER WAY 
  >> (setq arbol (id3-tree *training.examples* *target.attribute* *attributes*)))
  >> (setq arbol (random-tree *training.examples* *target.attribute* *attributes*)))


  ;; LOOK AT THE TREE IN A MORE HUMAN-FRIENDLY FORM
  >> (print.tree tree)
  OUTLOOK
   = SUNNY
       HUMIDITY
        = NORMAL => YES
        = HIGH => NO
   = OVERCAST => YES
   = RAIN
       WIND
        = STRONG => NO
        = WEAK => YES

  ;; FINALLY, USE THIS LEARNED TREE TO CLASSIFY AN INSTANCE
  >> (classify 'd6 tree)
  NO

|#
;;;; Funciones de acceso a ejemplos en listas de propiedad 
;;;; cada ejemplo se identifica con un simbolo Lisp (p.ej. X1, x233, ...)
;;;; y cada atributo se almacena como una propiedad de dicho simbolo 
;;;;

(defun put.value (attribute instance val)
  "Asigna un valor a un determinado atributo de un ejemplo o caso"
  (setf (get instance attribute) val))

(defun get.value (attribute instance)
  "Obtiene el valor que tiene un ejemplo o caso para un determinado attributo"
  (get instance attribute))

(defun print.entity (instance)
  "Imprime un ejemplo o caso, es decir, los valores de todos sus atributos"
  (print (symbol-plist instance)))

;;;;
;;;; Algunas variables globables y funciones de lectura y carga de datos en memoria
;;;;

(defvar *data* nil)
(defvar *data.name* nil)
(defvar *target.attribute* nil)
(defvar *training.examples* nil)

(defun inicializar.datos (input.file)
  "Inicializar ejemplos o casos a partir del fichero input.file,
   asignando valores a variables globales"
  (setq *training.examples* nil)  ;;; Lista de ejemplos leidos
  (setq *target.attribute* nil)   ;;; Clase, o atributo a aprender/predecir
  (setq *attributes* nil)         ;;; Lista de atributos de los ejemplos
  (setq atributos nil)
  (setq *data* nil)
  (setq *data.name* nil)
  (with-open-file (filein input.file :direction :input)
     (setq *data.name*  (read filein))
     (do ((dato (read filein nil) (read filein nil)))
	 ((listp dato) (setq *data* (append *data* (list dato))))
	 (setq atr nil)
	 (setq atr (append atr (list dato)))
	 (do ((dato (read filein nil) (read filein nil)))
	     ((equal dato '*))
	     (setq atr (append atr (list dato)))
	     )
	 (setq atributos (append atributos (list atr)))
      )
     (do ((dato (read filein nil) (read filein nil)))
	 ((equal dato '*))
	 (setq *data* (append *data* (list dato)))
      )
   )
  (dolist (d *data*) 
     (let ((value d))
      (setf *training.examples* (cons (first d) *training.examples*))
      (dolist (attribute atributos)
            (setq value (cdr value))
            (put.value (first attribute) (first d) (car value)))))

  (put.value 'legal.values (first (first (last atributos))) 
                           (rest  (first (last atributos))))
  (setq *target.attribute* (first (first (last atributos))))
  (do ((atrio atributos (setq atrio (rest atrio))))
       ((equal *target.attribute* (first (first atrio))) t)
       (setq *attributes* (append *attributes* (list (first (first atrio)))))
       )
)


; ***********************
; Representacion del Arbol: cada nodo interior (no hoja) es una lista de la forma
;  (atributo (valor1 subarbol1)(valor2 subarbol2)...)
;  donde subarbol-n puede ser otro nodo interior, o un nodo terminal (hoja) que
;  consiste en el valor de la clase asociada con los ejemplos que correspondan al
;  camino que lleva hasta esta hoja.

(defun id3-tree (examples target.attribute attributes)
  "TARGET.ATTRIBUTE es el atributo a aprender o predecir con el arbol que
   se va a construir.  EXAMPLES es la lista de ejemplos de entrenamiento
   ATTRIBUTES  es una lista de atributos que podran ser utilizados en la 
   construccion del arbol (aqui no se incluye el TARGET.ATTRIBUTE)"
  ;;; debe construirse en la segunda parte de la practica 4
)

(defun random-tree (examples target.attribute attributes)
  "TARGET.ATTRIBUTE es el atributo a aprender o predecir con el arbol que
   se va a construir.  EXAMPLES es la lista de ejemplos de entrenamiento
   ATTRIBUTES  es una lista de atributos que podran ser utilizados en la
   construccion del arbol (aqui no se incluye el TARGET.ATTRIBUTE)"
  (let (firstvalue a partitions)
    (setq firstvalue (get.value target.attribute (first examples)))
    (cond
     ;; si todos ejemplos de la mismas clase: nodo hoja con esa clase
     ((every #'(lambda(e)(eq firstvalue (get.value target.attribute e))) examples)
      firstvalue)
     ;; si no hay mas atributos, nodo hoja con clase mas frecuente
     ((null attributes)
      (most.common.value target.attribute examples))
     ;; ELSE, escoger un atributo para particionar los datos de entrenamiento
     ;; y llamar a tree recursivamente a Tree para expandir los subarboles
     ;; por debajo de este nodo
     (t
      (setq partitions
            (mapcar #'(lambda (a) (partition a examples)) attributes) )
      (setq a (choose.random.attribute target.attribute partitions))
      (cons (first a)
            (mapcar #'(lambda (branch)
                           (list (first branch)
                                 (random-tree (cdr branch)
                                      target.attribute
                                      (remove (first a) attributes))) )
                    (cdr a)
            ))))
   ))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Eleccion aleatoria de un atributo al azar para colocarlo en la raiz
;;;
(defun choose.random.attribute (target.attribute partitions)
    (nth  (random (length partitions)) partitions)
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(defun partition (attribute instances)
  "Devuelve la particion de INSTANCES con respecto a los valores de ATTRIBUTE,
   con formato   (atributo (valor1 ej11 ej12 ...)(valor2 ej21 ej22 ...)...)"
  (let (result vlist v)
    (dolist (e instances)
	  (setq v (get.value attribute e))
	  (if (setq vlist (assoc v result))
	    (rplacd vlist (cons e (cdr vlist)))
	    (setq result (cons (list v e) result))))
    (cons attribute result)))


(defun most.common.value (attribute instances)
  (let ((long 0) longest)
    (dolist (p (rest (partition attribute instances)))
	  (when (> (length p) long)
            (setq long (length p))
            (setq longest p)))
    (car longest)))



;;;;
;;;; Algunas funciones auxiliares muy utiles
;;;;

(defun print.tree (tree &optional (depth 0))
  (dotimes (i depth) (format t " "))
  (format t "~A~%" (first tree))
  (dolist (subtree (cdr tree))
        (dotimes (i (+ depth 1)) (format t " "))
        (format t "= ~A" (first subtree))
        (if (atom (second subtree))
          (format t " => ~A~%" (second subtree))
          (progn (terpri)(print.tree (second subtree) (+ depth 5))))))

(defun classify (instance tree)
  "Clasifica un ejemplo mediante el arbol dado"
  (let (val branch)
    (if (atom tree) (return-from classify tree))
    (setq val (get.value (first tree) instance))
    (setq branch (second (assoc val (cdr tree))))
    (classify instance branch)))

(defun eval.fc (tree target.attribute)
   "Funcion de evaluacion, devuelve cuantos ejemplos de entrenamiento
    fueron clasificados correctamente por el arbol construido"
  (let ((count 0))
	(dolist (d *training.examples*)
	   (if (equal (classify d tree) (get.value target.attribute d))
		(setq count (1+ count))
	    )
	)
   count
   )
)


(defun random.sort (l)
  "reordenar aleatoriamente la lista l, sin modificar l"
  (mapcar #'car 
     (sort (mapcar #'(lambda (x) (list x (random (* 1000 (length l))))) l) 
           #'< :key #'cadr
     )))

