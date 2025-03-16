import requests as req # biblioteca necessária para realizar conexões HTTP
import datetime as dt # biblioteca necessária para utilização de timestamps
import time # biblioteca necessária para realizar atrasos no código
import os # biblioteca necessária para obtenção do caminho de salvamento dos arquivos txt

timestamp = dt.datetime.now() # obtém o timestamp atual da execução do script
horaformatada = timestamp.strftime("%A %d %B %y") # formata o time stamp para uma melhor nomeação dos arquivos txt
dir_path = os.getcwd() + "/ArquivosPY/HTTP" # obtém o caminho do dirétorio de trabalho atual do python, e aponta para uma pasta específica dele para armazenamento dos txt's
print(dir_path)

# função que realiza a conexão com o servidor HTTP e obtem os dados das rotas GET, juntamente com o salvamento deles em arquivos txt
def receber():
    while True:
        horario = dt.datetime.now()
        x = req.get("http://192.168.0.103/dados") # rota do dado atual (o endereço pode mudar conforme a rede conectada)
        y = req.get("http://192.168.0.103/porcento") # rota da porcentagem atual (o endereço pode mudar conforme a rede conectada)
        z = req.get("http://192.168.0.103/reserva") # rota para informar se a capacidade esta na reserva (o endereço pode mudar conforme a rede conectada)
        f = open((dir_path + "/" + horaformatada +"_dados" + ".txt"), "a") # criação/abertura do arquivo txt que receberá o dado atual
        g = open((dir_path + "/" + horaformatada +"_porcentagem" + ".txt"), "a") # criação/abertura do arquivo txt que receberá a porcentagem atual
        conteudodado = "<" + str(horario) + "> " + x.text + "g" # realiza a formatação do dado com a inclusão de um timestamp antes
        conteudoporcento = "<" + str(horario) + "> " + y.text + "%" # realiza a formatação da porcentagem com a inclusão de um timestamp antes
        f.write(conteudodado+"\n") # realiza a escrita no arquivo txt dos dados
        g.write(conteudoporcento+"\n") # realiza a escrita no arquivo txt das porcentagens
        f.close()
        g.close()
        print("Dado recebido da balança é: " + x.text + "g.")
        print("Porcentagem recebida da balança é: " + y.text + " %.\n")
        if(z.text == "1"): # se o conteúdo exibido na rota 'reserva' for 1, avisa o usuário
            print("CHEGOU NA RESERVA")  
        time.sleep(10) # realiza a leitura e salvamento a cada 10 segundos 

# realiza a execução da função 'receber' até que haja uma interrupção
def run():
    try:
        receber() 
    except KeyboardInterrupt:
        print("Programa Finalizado.")
        raise SystemExit
        

if __name__ == '__main__':
    run()