;; Ingenieria del Conocimiento. Curso 2005/2006.
;; Practica 4: adquisicion automatica de conocimiento en reglas (aprendizaje automatico).
;; Parte 3: creacion de arboles ID3.
;; Autores: Juan Navarro Moreno
;;          Antonio Gonzalez Huete
;;          Grupo MD, pareja 06
;;
;; Notas: Revision completa de las practicas para el examen de Septiembre.


;; Cambia el directorio de trabajo.
(setf *default-pathname-defaults* (pathname (chdir "h:/ic/p4/")))

;;;; Carga las partes 0, 1 y 2.
(load "P42-simplificacion.lisp")


(defun id3-tree (examples target.attribute attributes)
  "TARGET.ATTRIBUTE es el atributo a aprender o predecir con el arbol que
se va a construir.  EXAMPLES es la lista de ejemplos de entrenamiento.
ATTRIBUTES  es una lista de atributos que podran ser utilizados en la
construccion del arbol (aqui no se incluye el TARGET.ATTRIBUTE)."
  (let (firstvalue a partitions)
    (setq firstvalue (get.value target.attribute (first examples)))
    (cond
     ;; Si todos ejemplos de la mismas clase: nodo hoja con esa clase.
     ((every #'(lambda(e)(eq firstvalue (get.value target.attribute e))) examples)
      firstvalue)
     ;; Si no hay mas atributos, nodo hoja con clase mas frecuente.
     ((null attributes)
      (most.common.value target.attribute examples))
     ;; ELSE, escoger un atributo para particionar los datos de entrenamiento
     ;; y llamar a random-tree recursivamente para expandir los subarboles
     ;; por debajo de este nodo.
     (t
      (setq partitions
            (mapcar #'(lambda (a) (partition a examples)) attributes))
      (setq a (choose.best.attribute target.attribute partitions examples))
      (cons (first a)
            (mapcar #'(lambda (branch)
                        (list (first branch)
                              (id3-tree (cdr branch)
                                        target.attribute
                                        (remove (first a) attributes))))
              (cdr a) )))) ))


(defun choose.best.attribute (target.attribute partitions examples)
  "Eleccion del atributo que provea de la mayor ganancia de informacion."
  ;; Calcula la entropia global.
  (let ((entropia-global (entropia-binaria target.attribute examples))
        (entropia-local 0)
        (ganancia-info 0)
        (mejor-ganancia 0)
        mejor-particion)
    ;; Calcula la ganancia de informacion para cada atributo.
    (dolist (p partitions mejor-particion)
      (setq ganancia-info 0)
      (setq entropia-local 0)
      (dolist (atr (cdr p))
        ;; atr recorre cada atributo de p, por ejemplo:
        ; (cdr p) == ((PESADO X1 X2 X3) (LIGERO X7 X8 X9))
        (setq entropia-local (+ entropia-local
                                (* (/ (length (cdr atr))
                                      (length examples))
                                   (entropia-binaria target.attribute (cdr atr))))))
      (setq ganancia-info (- entropia-global entropia-local))
      (when (> ganancia-info mejor-ganancia)
        (setq mejor-ganancia ganancia-info)
        (setq mejor-particion p)))))


(defun entropia-binaria (target.attribute examples)
  "Calcula la entropia binaria de los ejemplos respecto del objetivo."
  (let* ((part (partition target.attribute examples))
         (p (/ (length (cdr (second part))) (length examples)))
         (q (/ (length (cdr (third part))) (length examples))))
    (if (or (<= p 0) (<= q 0))
        0
      (- (+ (* p (log p 2)) (* q (log q 2)))))))

