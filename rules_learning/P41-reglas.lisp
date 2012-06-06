;; Ingenieria del Conocimiento. Curso 2005/2006.
;; Practica 4: adquisicion automatica de conocimiento en reglas (aprendizaje automatico).
;; Parte 1: generacion y evaluacion de reglas a partir de arboles.
;; Autores: Juan Navarro Moreno
;;          Antonio Gonzalez Huete
;;          Grupo MD, pareja 06
;;
;; Notas: Revision completa de las practicas para el examen de Septiembre.
;;        Este fichero es cargado automaticamente por la parte 2 de la practica.


;;;; Carga la parte 0.
(load "P40-random-trees.lisp")


;;;; Funciones basicas de acceso a hechos, reglas y predicciones.

(defun pred (fact) (first fact))
(defun valor (fact) (second fact))

(defun rule-ifs (r) (third r))
(defun rule-thens (r) (fifth r))
(defun rule-prediction (r) (valor (first (rule-thens r))))

(defun target-values () (get.value 'legal.values *target.attribute*))



;;;; Generacion de reglas a partir de arboles
;;;; ----------------------------------------

(defun rules-from-tree (tree)
  "Construye un conjunto de reglas a partir de un arbol de decision.
NOTA: smallest-ruleset es aplicable cuando solo hay dos posibles target-vaules."
  (smallest-ruleset
   (rules-from-paths
    (paths tree))))


(defun paths (tree)
  "Encuentra todos los posibles caminos que recorren un arbol desde
la raiz hasta las hojas."
  (if (member tree (target-values))
      (list (list (list *target.attribute* tree)))
    (mapcan #'(lambda (x)
                (mapcar #'(lambda (y) (cons (list (car tree) (car x)) y))
                  (paths (second x))))
      (cdr tree))
    ))


(defun rules-from-paths (paths)
  "Convierte un conjunto de caminos en un conjunto de reglas."
  (mapcar #'(lambda (x) (list (gentemp "R") 'IF (butlast x) 'THEN (last x))) paths))


(defun smallest-ruleset (ruleset)
  "Separa las reglas en dos grupos segun su consecuente, y devuelve el grupo
con menor numero de reglas."
  (let (ruleset1 ruleset2 target1)
    (setq target1 (rule-prediction (first ruleset)))
    (setq ruleset1 (mapcan #'(lambda (x)
                               (when (equal target1 (rule-prediction x)) (list x)))
                     ruleset))
    (setq ruleset2 (mapcan #'(lambda (x)
                               (unless (equal target1 (rule-prediction x)) (list x)))
                     ruleset))
    (if (< (length ruleset1) (length ruleset2))
        ruleset1
      ruleset2)))




;;;; Clasificacion y evaluacion de reglas
;;;; ------------------------------------

(defun prediction-stats (instances ruleset)
  "Devuelve el porcentaje de ejemplos de la lista de ejemplos dada
para los que el conjunto de reglas dada produce una prediccion correcta."
  (let ((counter 0))
    (dolist (inst instances)
      (if (correct-prediction inst ruleset)
          (setq counter (1+ counter))))
    ;(format t "Hay ~A correctas de un total de ~A~%" counter (length instances))
    (* (/ counter (length instances)) 100.0)))


(defun correct-prediction (instance ruleset)
  "Devuelve T solo si la clasificacion propia del ejemplo dado
es la misma que la obtenida utilizando el conjunto de reglas dado."
  ;(format t "Atributo ~A, classify da ~A, el atributo es ~A~%" instance (classify-with-ruleset instance ruleset) (get.value *target.attribute* instance))
  (equal (classify-with-ruleset instance ruleset)
         (get.value *target.attribute* instance)))


(defun classify-with-ruleset (instance ruleset)
  "Obtiene el valor de clasificacion que el conjunto de reglas dado
le asignaria al ejemplo dado, si es que dicho ejemplo se puede clasificar
con alguna de las reglas de ese conjunto; y en caso contrario, no devuelve
NIL, sino que devuelve la clasificacion opuesta correspondiente."
  (let (opuesto clase)
    (setq opuesto (if (equal (first (target-values)) (rule-prediction (first ruleset)))
                      (second (target-values))
                    (first (target-values))))
    (loop
      (cond
       ((null ruleset)
        (return opuesto))
       ((setq clase (classify-with-rule instance (car ruleset)))
        (return clase))
       (t
        (setq ruleset (cdr ruleset)))))))


(defun classify-with-rule (instance rule)
  "Obtiene el valor de clasificacion que la regla dada le asignaria
al ejemplo dado si es que dicho ejemplo cumple todas las premisas
de esa regla; o bien, devuelve NIL en caso contrario."
  (if (satisfies-premises? instance (rule-ifs rule))
      (rule-prediction rule)))


(defun satisfies-premises? (instance premises)
  "Comprueba que el ejemplo dado cumpla todas las premisas dadas."
  (cond
   ((null premises) t)
   ((satisfies-premise? instance (car premises))
    (satisfies-premises? instance (cdr premises)))
   (t nil)))


(defun satisfies-premise? (instance premise)
  "Comprueba que el ejemplo dado cumpla una determinada premisa."
  (equal (get.value (pred premise) instance) (valor premise)))

