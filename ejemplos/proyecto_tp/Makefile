# Makefile Principal Dinámico

# Detectar librerías en libs/
LIB_DIRS := $(wildcard libs/*)

# Detectar ejercicios en ejercicios/
EX_DIRS := $(wildcard ejercicios/*)

.PHONY: all librerias clean run test memcheck fallos $(EX_DIRS) $(LIB_DIRS)

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

# Corre el objetivo $(1) en cada librería y ejercicio sin cortar en la primera
# suite que falla: al final lista las que fallaron y sale con 1.
define recorrer_suites
	@fallas=""; \
	for dir in $(LIB_DIRS) $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- $(2) $$dir ---"; \
			$(MAKE) -C $$dir $(1) </dev/null || fallas="$$fallas $$dir"; \
		fi; \
	done; \
	if [ -n "$$fallas" ]; then \
		echo "Fallaron:$$fallas"; \
		exit 1; \
	fi; \
	echo "Todas las suites terminaron bien."
endef

# Una librería que no compila no impide probar el resto (-k)
test:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,test,Probando)

memcheck:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,memcheck,Memcheck en)

# Inyección de fallos con vasquez (cada suite la saltea si no está instalado)
fallos:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,fallos,Fallos en)

clean:
	@echo "Limpiando todos los ejecutables, librerías estáticas y archivos objeto..."
	@for dir in $(LIB_DIRS) $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done

# Incluir personalizaciones locales si existen
-include local.mk
