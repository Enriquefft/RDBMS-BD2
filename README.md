# RDBMS-BD2
Relationadatabase managment system, made in c++ for Database 2

## Tabla de Contenidos

- [RDBMS-BD2](#rdbms-bd2)
  - [Tabla de Contenidos](#tabla-de-contenidos)
  - [Objetivos](#objetivos)
    - [Principal](#principal)
    - [Secundarios](#secundarios)
  - [Organización de archivos](#organización-de-archivos)
  - [Funciones implementadas](#funciones-implementadas)
  - [Dataset](#dataset)
    - [Descripción de las Columnas](#descripción-de-las-columnas)
  - [SQLParser](#sqlparser)
    - [Queries](#queries)
    - [Non terminals](#non-terminals)
  - [Experimentación](#experimentación)
  - [Conclusiones](#conclusiones)
  - [Autores](#autores)
  - [Bibliografía](#bibliografía)

## Objetivos

### Principal

- Desarrollar un sistema de gestión de bases de datos relacionales que emplea índices basados en estructuras de datos reconocidas, almacenados en archivos, con el objetivo de mejorar el rendimiento de las consultas y comprender su comportamiento en función de diversas cantidades de datos.

### Secundarios

- Armar un parser para el lenguaje SQL que permita realizar operaciones de carga de archivos, creación de tablas , creación de índices y consulta de datos.

- Comparación de estructuras de datos: Realizar un análisis comparativo sobre las estructuras de datos utilizadas (AVL, ISAM, Sequential File) en términos de eficiencia, consumo de memoria y escalabilidad.
  
- Manejo de Memoria Secundaria: Explorar estrategias avanzadas para el manejo eficiente de la memoria secundaria, como la implementación de técnicas de paginación o buffering para minimizar los accesos a disco.

## Organización de archivos

Estrategias utilizadas en el presente proyecto:

* [Sequential Index](https://github.com/nicolas-castaneda/sequential-index)
* [AVL Index](https://github.com/AaronCS25/avl-index-zzz)
* [ISAM Index]()

## Funciones implementadas

Las estructuras de datos implementadas deben poder realizar las siguientes operaciones:

* Response add (T key);
* Response erase (T key);
* Response search(T key);
* Response searchByRange(T key);

**Nota:** Por defecto, todas las operaciones sobre la tabla se ejecutan sobre el índice Sequential, que se crea automáticamente para todos los índices. Esto significa que las operaciones sobre la tabla utilizarán el índice Sequential a menos que se especifique lo contrario.

## Dataset

El conjunto de datos utilizado en este proyecto proviene de la plataforma Kaggle y consiste en pistas de Spotify que abarcan diferentes géneros musicales. Este dataset es una **buena** elección por las siguientes razónes:

- **Variedad de Datos:** El dataset contiene una amplia gama de tipos de datos relacionados con la música, como detalles de las pistas, información de los artistas y atributos musicales. Esto proporciona una riqueza de información que permite probar la DBMS y su manejo de tablas con tipos de datos variados (*varchar, int, double, text, bool, etc*).

- **Calidad de los Datos:** Los datos de Spotify son de alta calidad y reflejan una representación real de la información que se encuentra en la plataforma de streaming de música. Esto hace que el dataset sea adecuado para simular una evaluación del rendimiento de las operaciones sobre datos reales en un contexto real.

- **Gran Volumen de Datos:** Con más de 114,000 observaciones, este dataset es lo suficientemente grande como para probar y evaluar el rendimiento de las operaciones de bases de datos con grandes cantidades de datos. Esto es fundamental para comprender cómo las estructuras de datos utilizadas en el proyecto se comportan en escenarios de poca, media y alta carga.

### Descripción de las Columnas

| Columna          | Descripción |
|------------------|---------------------------------------------------------------------------------------------------|
| track_id         | ID de Spotify para la pista.|
| artists          | Nombres de los artistas que interpretaron la pista (separados por ; si hay más de uno).|
| album_name       | Nombre del álbum en el que aparece la pista.|
| track_name       | Nombre de la pista.|
| popularity       | Popularidad de la pista (valor entre 0 y 100, calculado en función de reproducciones y precencia). |
| duration_ms      | Duración de la pista en milisegundos.|
| explicit         | Indica si la pista tiene letras explícitas (true = sí tiene; false = no tiene O desconocido).|
| danceability     | Adecuación de la pista para bailar (0.0 menos bailable, 1.0 muy bailable).|
| energy           | Medida de intensidad y actividad de la pista (0.0 a 1.0).|
| key              | Tonalidad de la pista (números enteros mapeados a notas musicales).|
| loudness         | Volumen general de la pista en decibeles (dB).|
| mode             | Modalidad de la pista (1 para mayor, 0 para menor).|
| speechiness      | Detecta presencia de palabras habladas (0.0 a 1.0).|
| acousticness     | Medida de confianza de si la pista es acústica (0.0 a 1.0).|
| instrumentalness | Predice si la pista no contiene vocales (0.0 a 1.0).|
| liveness         | Detecta presencia de audiencia (0.0 a 1.0, mayor valor indica interpretación en vivo).|
| valence          | Positividad musical transmitida por la pista (0.0 a 1.0).|
| tempo            | Tempo estimado en BPM (ritmo de la pista).|
| time_signature   | Firma de tiempo estimada (3 a 7 indica compás de 3/4 a 7/4).|
| track_genre      | Género al que pertenece la pista.|


## SQLParser



### Queries

### Non terminals

## Experimentación

Para probar el funcionamiento del proyecto se realizó las siguientes queries:

´´´sql
CREATE TABLE test(id int primary key, col1 char(50), mode int, val double);

´´´


## Conclusiones

* Conclusión 1
* Conclusión 2
* Conclusión 3
* Conclusión 4
* Conclusión 5

## Autores

| **Aaron Camacho** | **Nicolas Castañeda** | **Juaquín Remon** | **Enrique Flores** | **Renato Cernades** |
|:------------:|:------------:|:------------:|:------------:|:------------:|
| ![AaronCS25](https://avatars.githubusercontent.com/u/102536323?s=400&v=4) | ![nicolas-castaneda](https://avatars.githubusercontent.com/u/102196795?v=4) | ![jauquin456](https://avatars.githubusercontent.com/u/83974317?v=4) | ![Enriquefft](https://avatars.githubusercontent.com/u/60308719?v=4) | ![Avatar del Autor 5](URL_del_Avatar_Autor_5) |
| [github.com/AaronCS25](https://github.com/AaronCS25) | [github.com/NickCQCCS](https://github.com/nicolas-castaneda) | [github.com/juaquin456](https://github.com/juaquin456) | [github.com/Enriquefft](https://github.com/Usuario_Autor_4) | [github.com/Usuario_Autor_5](https://github.com/Usuario_Autor_5) |

## Bibliografía

- [1] ["simple_wc_example"](https://github.com/jonathan-beard/simple_wc_example). Disponible en GitHub. [Accedido: 23 Sep, 2023]
- [2] ["Spotify Tracks Dataset"](https://www.kaggle.com/datasets/maharshipandya/-spotify-tracks-dataset). Disponible en Kaggle. [Accedido: 23 Sep, 2023]
- [3] ["Introduction to AVL Tree"](https://www.geeksforgeeks.org/introduction-to-avl-tree/). Disponible en GeeksforGeeks. [Accedido: 23 Sep, 2023]
- [4] ["File Organization in DBMS"](https://www.geeksforgeeks.org/file-organization-in-dbms-set-1/). Disponible en GeeksforGeeks. [Accedido: 23 Sep, 2023]
- [5] ["Why use a .tpp file when implementing templated functions and classes defined in a .h file?"](https://stackoverflow.com/questions/44774036/why-use-a-tpp-file-when-implementing-templated-functions-and-classes-defined-i). Disponible en StackOverflow. [Accedido: 23 Sep, 2023]
