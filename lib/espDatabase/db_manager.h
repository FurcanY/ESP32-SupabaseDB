#ifndef DB_MANAGER_H

#define DB_MANAGER_H


#include "esp_err.h"
#include "config.h"




/**
  * @brief Non Volitale Storage içerisinde veri kaydetmeyi sağlar.
  * @brief Database API, HTTP Gönderimi yapar.
  *         
  *     key   : saklanacak verinin anahtarı
  *     value : saklanacak veri
  * 
  *     Bool olarak veri saklanır.
  *     
  */
void update_variable_bool(const char* key, bool value);




#endif // DB_MANAGER_H



