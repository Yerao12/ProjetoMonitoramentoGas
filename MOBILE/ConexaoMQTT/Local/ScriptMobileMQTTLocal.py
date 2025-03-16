import datetime as dt # biblioteca necessária para utilização de timestamps
from paho.mqtt import client as mqtt_client # biblioteca necessária para realizar conexões MQTT
import os # biblioteca necessária para obtenção do caminho de salvamento dos arquivos txt

broker = '192.168.0.103' # endereço IPv4 do broker na rede
port = 1883 # porta de utilização do MQTT, neste caso a padrão
client_id = "SMARTPHONE_YERRO" # como o dispositivo que executa este script irá se identificar no broker
timestamp = dt.datetime.now() # obtém o timestamp atual da execução do script
horaformatada = timestamp.strftime("%A %d %B %y") # formata o time stamp para uma melhor nomeação dos arquivos txt
dir_path = os.getcwd() + "/ArquivosPY/MQTTLocal" # obtém o caminho do dirétorio de trabalho atual do python, e aponta para uma pasta específica dele para armazenamento dos txt's
print(dir_path)

# função que realiza a conexão ao broker
def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Conectado ao broker!")
        else:
            print("Falha ao conectar, com codigo %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

# função que realiza a inscrição aos tópicos 'dados','porcento' e 'reserva'
def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg): # função de retorno, que realiza manipulações com os dados recebidos dos tópicos inscritos
        horario = dt.datetime.now()
        if(msg.topic == "dados"): # caso o dado seja do tópico 'dados', realiza salvamento dele em um txt próprio
            f = open((dir_path + "/" + horaformatada +"_dados" + ".txt"), "a") # criação/abertura do arquivo txt que receberá o dado atual
            conteudodado = str(horario) + " " + msg.payload.decode() + "g" # formatação do dado antes de ser escrito no arquivo, incluindo um timestamp
            f.write(conteudodado+"\n")
            f.close()
            print("Dado recebido da balança é: " + msg.payload.decode() + "g.")
        
        if(msg.topic == "porcento"): # caso o dado seja do tópico 'porcentagem', realiza salvamento dele em um txt próprio
            g = open(dir_path + "/" + horaformatada +"_porcentagem" + ".txt","a") # criação/abertura do arquivo txt que receberá a porcentagem atual
            conteudoporcento = str(horario) + " " + msg.payload.decode() + "%" # formatação do dado antes de ser escrito no arquivo, incluindo um timestamp
            g.write(conteudoporcento+"\n")
            g.close()
            print("A porcentagem recebida da balança é: " + msg.payload.decode() + " %.\n")   

        if(msg.topic == "reserva"): # caso o dado seja do tópico 'reserva', realiza o print dele
            print(msg.payload.decode() + "\n")

    client.subscribe("dados",0) # inscrição no tópico 'dados', com a qualidade de serviço 0
    client.subscribe("porcento",0) # inscrição no tópico 'porcento', com a qualidade de serviço 0
    client.subscribe("reserva",0) # inscrição no tópico 'reserva', com a qualidade de serviço 0
    client.on_message = on_message # definição da função que irá trabalhar com os dados recebidos

# realiza a criação de um objeto client, que realiza a função de conexão ao broker e se inscreve aos tópicos, ficando em loop até que haja uma interrupção
def run():
    try:
        client = connect_mqtt()
        subscribe(client)
        client.loop_forever()
    except KeyboardInterrupt:
        print("Programa Finalizado.")
        raise SystemExit
        


if __name__ == '__main__':
    run()
