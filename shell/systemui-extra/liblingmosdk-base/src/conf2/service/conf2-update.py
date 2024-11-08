#!/usr/bin/python3.8
import os
import logging
import conf2Utils

os.makedirs('/etc/lingmo-config', exist_ok=True)
logging.basicConfig(filename='/etc/lingmo-config/systembus.log', 
                    level=logging.DEBUG, 
                    format='%(asctime)s.%(msecs)03d - %(name)s - %(levelname)s - %(message)s', 
                    datefmt='%Y-%m-%d %H:%M:%S')
logger = logging.getLogger()
converter = conf2Utils.Converter(logger)

logger.info('update db start')
if converter.update_read_only_db():
    # converter.update_user_db('/root/.config/lingmo-config/user.db')
    pass
logger.info('update db end')
