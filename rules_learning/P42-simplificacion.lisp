;; Ingenieria del Conocimiento. Curso 2005/2006.
;; Practica 4: adquisicion automatica de conocimiento en reglas (aprendizaje automatico).
;; Parte 2: simplificacion de reglas.
;; Autores: Juan Navarro Moreno
;;          Antonio Gonzalez Huete
;;          Grupo MD, pareja 06
;;
;; Notas: Revision completa de las practicas para el examen de Septiembre.
;;        Este fichero es cargado automaticamente por la parte 3 de la practica.


;;;; Carga las partes 0 y 1.
(load "P41-reglas.lisp")


(defun simplify-ruleset (ruleset &optional (rule-num 0))
  "Elimina reglas redundantes en un conjunto de reglas.
El contador rule-num indica el numero de regla que analizar."
  (cond
   ((= rule-num (length ruleset))
    ruleset)
   (t
    (let ((rule (nth rule-num ruleset)))
      (setq rule (best-premises rule ruleset))
      ;(format t "best-premises devuelve: ~A~%" rule)
      ;; Elimina redundantes y reinserta la regla en el conjunto.
      (setq ruleset (cons rule (remove-redundants rule ruleset)))
      (simplify-ruleset ruleset (1+ rule-num))))))


(defun remove-redundants (rule ruleset)
  "Limpia el conjunto de reglas dado de aquellas que sean redundantes,
iguales o mas especificas que la relga dada."
  (mapcan #'(lambda (x)
              (unless (and (equal (rule-thens rule) (rule-thens x))
                           (subsetp (rule-ifs rule) (rule-ifs x) :test #'equal))
                (list x)))
    ruleset))


(defun best-premises (rule ruleset)
  "Busca la combinacion con el menor numero de premisas posible, que no
empeore los resultados (la tasa de aciertos) del conjunto de reglas original."
  (declare (optimize (speed 3)))
  (let ((ifs (rule-ifs rule))
        (stats (prediction-stats *training.examples* ruleset))
        aux-rule
        aux-ruleset
        aux-stats)
    
    ;(format t "rule: ~A~%" rule)
    
    (dolist (aux-ifs (genera-permutaciones ifs) rule)
      
      ;; Si solo queda una premisa, no podemos mejorarla mas.
      (when (= 1 (length (rule-ifs rule)))
        ;(format t "salgo porque solo queda una premisa~%")
        (return rule))
      
      ;; Solo comprueba las aux-ifs si son menos de las que ya teniamos.      
      (when (< (length aux-ifs) (length (rule-ifs rule)))
        (setq aux-rule (substitute aux-ifs (rule-ifs rule) rule))
        ;(format t "aux-rule: ~A~%" aux-rule)
        ;; Para sustituir la regla antigua con la nueva, se fija en el numero de regla.
        (setq aux-ruleset (substitute aux-rule rule ruleset
                                      :test #'(lambda (x y) (equal (car x) (car y)))))
        ;(format t "aux-ruleset: ~A~%" aux-ruleset)
        (setq aux-stats (prediction-stats *training.examples* aux-ruleset))
        (unless (< aux-stats stats)
          ;(format t "Encontrada regla con menos premisas~%")
          (setq rule aux-rule)))
      )))


(defun genera-permutaciones (lista)
  (let (result)
    (cond
     ((null (cdr lista))
      (list lista))
     (t
      (setq result (genera-permutaciones (cdr lista)))
      (nconc result (mapcar #'(lambda (x) (cons (car lista) x)) result))
      (cons (list (car lista)) result)))))

