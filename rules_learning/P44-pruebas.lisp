;; Ingenieria del Conocimiento. Curso 2005/2006.
;; Practica 4: adquisicion automatica de conocimiento en reglas (aprendizaje automatico).
;; Ejemplos de prueba.
;; Autores: Juan Navarro Moreno
;;          Antonio Gonzalez Huete
;;          Grupo MD, pareja 06
;;
;; Notas: Revision completa de las practicas para el examen de Septiembre.


;; Cambia el directorio de trabajo.
(setf *default-pathname-defaults* (pathname (chdir "h:/ic/p4/")))


;; PRUEBAS PARTE 1
;; ---------------

(setf *target.attribute* 'TRATAMIENTO)
(put.value 'legal.values *target.attribute* '(OK RIESGO))
(setq bonzai '(FUNCIONRENAL
               (DEFICIENTE
                   (EMBARAZADA (NO OK)
                               (SI RIESGO)))
               (NORMAL
                (DIABETES (NO OK)
                          (SI RIESGO)))))
(paths bonzai)
(rules-from-tree bonzai)


(inicializar.datos "restaurant.dat")
(setq arbol (random-tree *training.examples* *target.attribute* *attributes*))
(setq reglas (rules-from-tree arbol))
(prediction-stats *training.examples* reglas)

(inicializar.datos "adicionales.dat")
(prediction-stats *training.examples* reglas)

(inicializar.datos "riesgo-set1.dat")
(setq arbol (random-tree *training.examples* *target.attribute* *attributes*))
(setq reglas (rules-from-tree arbol))
(prediction-stats *training.examples* reglas)

(inicializar.datos "riesgo-set2.dat")
(prediction-stats *training.examples* reglas)




;; PRUEBAS PARTE 2
;; ---------------

(inicializar.datos "restaurant.dat")
(setq arbol (random-tree *training.examples* *target.attribute* *attributes*))
(setq reglas1 (rules-from-tree arbol))
(setq reglas2 (simplify-ruleset reglas1)))
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

(inicializar.datos "adicionales.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)


;; AVISO: TARDA un rato (de media 20 seg. en un micro a 2,50 GHZ).
(inicializar.datos "riesgo-set1.dat")
(setq arbol (random-tree *training.examples* *target.attribute* *attributes*))
(setq reglas1 (rules-from-tree arbol))
(setq reglas2 (simplify-ruleset reglas1))
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

(inicializar.datos "riesgo-set2.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

(inicializar.datos "riesgo-set3.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)




;; PRUEBAS PARTE 3
;; ---------------

(inicializar.datos "restaurant.dat")
(setq arbol (id3-tree *training.examples* *target.attribute* *attributes*))
(setq reglas1 (rules-from-tree arbol))
(setq reglas2 (simplify-ruleset reglas1))

(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

(inicializar.datos "adicionales.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)


(inicializar.datos "riesgo-set1.dat")
(setq arbol (id3-tree *training.examples* *target.attribute* *attributes*))
(setq reglas1 (rules-from-tree arbol))
(time (setq reglas2 (simplify-ruleset reglas1)))

(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)


(inicializar.datos "riesgo-set2.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

(inicializar.datos "riesgo-set3.dat")
(prediction-stats *training.examples* reglas1)
(prediction-stats *training.examples* reglas2)

