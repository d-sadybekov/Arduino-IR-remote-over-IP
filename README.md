# Arduino-IR-remote-over-IP
IR control remote sattelite recievers (OTAU) over IP (LAN)

Сервер реализована W5100 + Uno. Поднимает вебсервер со страничкой с сохраненными командами от трех разных пультов ресиверов Otau TV.
Сервер сожидает подключения клиента для приема команд и адресов и отправку их на ИК трансмиттер.
Клиент принимает ИК команду с ПДУ, выдает в консоль декодированный адрес, протокол и команду.
Клиент соединяется с Сервером по IP, отсылает  GET-запрос с полученными с ПДУ командой и адресом. 
