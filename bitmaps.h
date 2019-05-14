#include <Arduino.h>

static const unsigned char o2_bits [] PROGMEM = {
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000001, 0b11111000, 0b00000000, 0b00000000, //        ######                   
  0b00000011, 0b11111110, 0b00000000, 0b00000000, //       #########                 
  0b00000111, 0b00000111, 0b00000000, 0b00000000, //      ###     ###                
  0b00001100, 0b00000011, 0b00000000, 0b00000000, //     ##        ##                
  0b00001100, 0b00000011, 0b00000000, 0b00000000, //     ##        ##                
  0b00011000, 0b00000001, 0b10000000, 0b00000000, //    ##          ##               
  0b00011000, 0b00000001, 0b11111000, 0b00000000, //    ##          ######           
  0b00011000, 0b00000001, 0b11111100, 0b00000000, //    ##          #######          
  0b00011000, 0b00000001, 0b10001110, 0b00000000, //    ##          ##   ###         
  0b00011000, 0b00000001, 0b10000110, 0b00000000, //    ##          ##    ##         
  0b00001100, 0b00000011, 0b00000110, 0b00000000, //     ##        ##     ##         
  0b00001100, 0b00000011, 0b00000110, 0b00000000, //     ##        ##     ##         
  0b00001110, 0b00001110, 0b00000100, 0b00000000, //     ###     ###      #          
  0b00000111, 0b11111100, 0b00001100, 0b00000000, //      #########      ##          
  0b00000001, 0b11111000, 0b00011000, 0b00000000, //        ######      ##           
  0b00000000, 0b00000000, 0b00110000, 0b00000000, //                   ##            
  0b00000000, 0b00000000, 0b01100000, 0b00000000, //                  ##             
  0b00000000, 0b00000000, 0b11000000, 0b00000000, //                 ##              
  0b00000000, 0b00000001, 0b10000000, 0b00000000, //                ##               
  0b00000000, 0b00000011, 0b11111111, 0b00000000, //               ##########        
  0b00000000, 0b00000011, 0b11111111, 0b00000000, //               ##########        
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //  
};

static const unsigned char batt_bits [] PROGMEM = {
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000001, 0b11110000, 0b00011111, 0b00000000, //        #####       #####        
  0b00000010, 0b00001000, 0b00100000, 0b10000000, //       #     #     #     #       
  0b00000010, 0b00001000, 0b00100000, 0b10000000, //       #     #     #     #       
  0b00000010, 0b00001000, 0b00100000, 0b10000000, //       #     #     #     #       
  0b00000010, 0b00001000, 0b00100000, 0b10000000, //       #     #     #     #       
  0b00000010, 0b00001000, 0b00100000, 0b10000000, //       #     #     #     #       
  0b00011111, 0b11111111, 0b11111111, 0b11110000, //    #########################    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010011, 0b11111000, 0b00111111, 0b10010000, //    #  #######     #######  #    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010000, 0b01000000, 0b00000000, 0b00010000, //    #     #                 #    
  0b00010000, 0b00000000, 0b00000000, 0b00010000, //    #                       #    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00011110, 0b00000000, 0b00000000, 0b11110000, //    ####                 ####    
  0b00011111, 0b11111111, 0b11111111, 0b11110000, //    #########################    
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
};

static const unsigned char oil_bits [] PROGMEM  = {
0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b01000000, 0b00001110, 0b00000000, 0b00000000, //  #          ###                 
  0b01100000, 0b00011111, 0b00000000, 0b11100000, //  ##        #####        ###     
  0b01111000, 0b00001110, 0b00000011, 0b10000000, //  ####       ###       ###       
  0b01111100, 0b00000100, 0b00001101, 0b00000000, //  #####       #      ## #        
  0b01111111, 0b11111111, 0b11111110, 0b01000000, //  ######################  #      
  0b00010111, 0b11111111, 0b11111110, 0b00010000, //    # ##################    #    
  0b00001111, 0b11111111, 0b11111100, 0b00010000, //     ##################     #    
  0b00001111, 0b11111111, 0b11111100, 0b00011000, //     ##################     ##   
  0b00000111, 0b11111111, 0b11111000, 0b00111100, //      ################     ####  
  0b00000111, 0b11111111, 0b11110000, 0b00111100, //      ###############      ####  
  0b00000111, 0b11111111, 0b11110000, 0b00111100, //      ###############      ####  
  0b00000111, 0b11111111, 0b11100000, 0b00011000, //      ##############        ##   
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b11011111, 0b11100000, 0b00000000, //         ## ########             
  0b00000000, 0b11011111, 0b11100000, 0b00000000, //         ## ########             
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011111, 0b11000000, 0b00000000, //            #######              
  0b00000000, 0b00011111, 0b11000000, 0b00000000, //            #######              
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                  
  0b00000000, 0b00011100, 0b00000000, 0b00000000, //            ###                                  
};

static const unsigned char coolant_bits [] PROGMEM  = {
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00111111, 0b10000000, 0b00000000, 0b00000000, //   #######                       
  0b00111000, 0b00000011, 0b10000000, 0b00000000, //   ###         ###               
  0b00111000, 0b00000011, 0b10000000, 0b00000000, //   ###         ###               
  0b00111000, 0b00000011, 0b11111110, 0b00000000, //   ###         #########         
  0b00111111, 0b00000011, 0b11111110, 0b00000000, //   ######      #########         
  0b00111000, 0b00000011, 0b11111110, 0b00000000, //   ###         #########         
  0b00111000, 0b00000011, 0b10000000, 0b00000000, //   ###         ###               
  0b00111000, 0b00000011, 0b10000000, 0b00000000, //   ###         ###               
  0b00111000, 0b00000011, 0b11111110, 0b00000000, //   ###         #########         
  0b00000000, 0b00000011, 0b11111110, 0b00000000, //               #########         
  0b00000000, 0b00000011, 0b11111110, 0b00000000, //               #########         
  0b00000000, 0b00000011, 0b10000000, 0b00000000, //               ###               
  0b00000000, 0b00000011, 0b10000000, 0b00000000, //               ###               
  0b00000000, 0b00000011, 0b10000000, 0b00000000, //               ###               
  0b00000000, 0b00000011, 0b10000111, 0b11110000, //               ###    #######    
  0b00000111, 0b11110011, 0b10011100, 0b00011100, //      #######  ###  ###     ###  
  0b00011100, 0b00011111, 0b11110000, 0b00000110, //    ###     #########         ## 
  0b01110000, 0b00000111, 0b11100000, 0b00000011, //  ###         ######           ##
  0b11000000, 0b00000011, 0b10000000, 0b00000001, // ##            ###              #
  0b10000000, 0b00000111, 0b11000000, 0b00000000, // #            #####              
  0b00000000, 0b00000111, 0b11000000, 0b00000000, //              #####              
  0b00000000, 0b00000111, 0b11000000, 0b00000000, //              #####              
  0b00000000, 0b00000111, 0b11000000, 0b00000000, //              #####              
  0b00000111, 0b11110011, 0b10000111, 0b11110000, //      #######  ###    #######    
  0b00011100, 0b00011100, 0b00011100, 0b00011100, //    ###     ###     ###     ###  
  0b01110000, 0b00000110, 0b01110000, 0b00000110, //  ###         ##  ###         ## 
  0b11000000, 0b00000011, 0b11000000, 0b00000011, // ##            ####            ##
  0b10000000, 0b00000001, 0b10000000, 0b00000001, // #              ##              #
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, //                                                               
};

static const unsigned char intake_bits [] PROGMEM  = {
  0x00, 0x00, 0x00, 0x00, 0x6f, 0x80, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x10, 0x00, 
  0x0f, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x3f, 0xe0, 0x0c, 0x00, 0x3f, 0xe0, 
  0x0c, 0xe0, 0x3f, 0xe0, 0x01, 0x10, 0x38, 0x00, 0x00, 0x10, 0x38, 0x00, 0x00, 0x10, 0x3f, 0xe0, 
  0x7f, 0xf3, 0x3f, 0xe0, 0x7f, 0xe4, 0xbf, 0xe0, 0x00, 0x00, 0xb8, 0x00, 0x1f, 0xff, 0xb8, 0x00, 
  0x1f, 0xff, 0x3f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 0x7f, 0xfe, 0x3f, 0xe0, 0x7f, 0xff, 0x38, 0x00, 
  0x00, 0x01, 0x38, 0x00, 0x00, 0x09, 0x7c, 0x00, 0x00, 0x06, 0x7c, 0x00, 0x00, 0x00, 0x7c, 0x00, 
  0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                                                                 
};