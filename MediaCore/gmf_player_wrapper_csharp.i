%module MediaCore
 %{
 /* Includes the header in the wrapper code */
 #include "gmf_player.h"
 %}
 
 /* Parse the header file to generate wrappers */
 %include "gmf_player.h"