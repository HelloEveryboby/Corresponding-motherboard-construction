#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

/**
 * @brief Initializes the NFC module and registers its command handlers.
 *
 * This function should be called once at startup. It registers the
 * handlers for NFC-related commands with the main command processor.
 */
void NFC_Init(void);


#endif // NFC_HANDLER_H
