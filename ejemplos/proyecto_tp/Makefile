# Makefile Principal Dinámico para Proyecto Ejemplo Basado en Plantilla-TP

# Detectar librerías en libs/
LIB_DIRS := $(wildcard libs/*)

# Detectar ejercicios en ejercicios/
EX_DIRS := $(wildcard ejercicios/*)

.PHONY: all librerias clean run test $(EX_DIRS) $(LIB_DIRS)

all: librerias $(EX_DIRS)

librerias: $(LIB_DIRS)

# Regla para compilar cada librería de forma independiente si tiene un Makefile
$(LIB_DIRS):
	@if [ -f $@/Makefile ]; then \
		echo "Compilando librería en $@..."; \
		$(MAKE) -C $@ || exit 1; \
	fi

# Regla para compilar cada ejercicio, asegurando que primero se compilen las librerías
$(EX_DIRS): librerias
	@if [ -f $@/Makefile ]; then \
		echo "Compilando ejercicio en $@..."; \
		$(MAKE) -C $@ || exit 1; \
	fi

run: librerias
	@echo "Ejecutando programas de ejercicios..."
	@for dir in $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- Ejecutando $$dir ---"; \
			$(MAKE) -C $$dir run || exit 1; \
		fi; \
	done

test: librerias
	@echo "Ejecutando pruebas de librerías..."
	@for dir in $(LIB_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- Probando librería $$dir ---"; \
			$(MAKE) -C $$dir test || exit 1; \
		fi; \
	done
	@echo "Ejecutando pruebas de ejercicios..."
	@for dir in $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- Probando ejercicio $$dir ---"; \
			$(MAKE) -C $$dir test || exit 1; \
		fi; \
	done

clean:
	@echo "Limpiando todos los ejecutables, librerías estáticas y archivos objeto..."
	@for dir in $(LIB_DIRS) $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done

-include local.mk
