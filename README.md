# Arquitetura de Monitoramento e Controle de Nível de Água com MQTT e ESP32

## Descrição do Projeto

Este projeto desenvolveu um sistema automatizado de monitoramento e controle de nível de água utilizando tecnologias IoT (Internet das Coisas). O sistema emprega um sensor ultrassônico para medir o nível da água em tempo real e aciona automaticamente uma válvula solenoide conforme a necessidade, com comunicação via protocolo MQTT para controle remoto.

**Objetivo:** Contribuir para o uso sustentável dos recursos hídricos através de uma solução tecnológica inovadora que otimiza o consumo, controle de qualidade e reuso da água, alinhado com o Objetivo de Desenvolvimento Sustentável 6 da ONU.

## Funcionalidades

- **Monitoramento em tempo real**: Medição contínua do nível de água a cada 500ms
- **Controle automático**: Acionamento automático da válvula quando o nível está baixo (>10cm)
- **Comunicação IoT**: Integração com broker MQTT para controle remoto
- **Interface web**: Monitoramento e controle via protocolo MQTT
- **Alertas**: Sistema de notificação baseado em thresholds configuráveis

## Como Reproduzir o Projeto

### Pré-requisitos

- **Hardware:**
  - 1x ESP32 DevKit V1
  - 1x Sensor Ultrassônico HC-SR04
  - 1x Módulo de Relé 5V (1 canal)
  - 1x Válvula solenoide 12V (opcional para testes)
  - Protoboard e jumpers
  - Fonte de alimentação 12V (para válvula)

- **Software:**
  - Arduino IDE
  - Bibliotecas: WiFi.h, PubSubClient.h
  - Acesso à internet para comunicação MQTT

### Montagem do Hardware

1. **Alimentação:**
   - VCC do HC-SR04 → 5V do ESP32
   - VCC do módulo relé → 5V do ESP32
   - GND de todos os componentes → GND do ESP32

2. **Sensor Ultrassônico HC-SR04:**
   - TRIG → GPIO 5
   - ECHO → GPIO 18

3. **Módulo de Relé:**
   - IN → GPIO 19
   - Conectar válvula solenoide aos contatos do relé

### Configuração do Software

1. **Instalação das bibliotecas:**
   ```cpp
   #include <WiFi.h>
   #include <PubSubClient.h>
   ```

2. **Configuração WiFi:**
   ```cpp
   const char* ssid = "SEU_WIFI";
   const char* password = "SUA_SENHA";
   ```

3. **Configuração MQTT:**
   ```cpp
   const char* mqtt_server = "broker.hivemq.com";
   const char* mqtt_topic_pub = "sensor/agua/nivel";
   const char* mqtt_topic_sub = "atuador/agua/valvula";
   ```

### Testando o Sistema

1. **Teste local:** Verifique as conexões e funcionamento básico
2. **Teste MQTT:** Utilize um cliente MQTT para enviar comandos:
   - Publique "1" no tópico `atuador/agua/valvula` para ligar a válvula
   - Publique "0" para desligar
3. **Monitoramento:** Assine o tópico `sensor/agua/nivel` para receber as medições

## Documentação do Software

### Estrutura do Código

```
sketch.ino
├── Configurações e definições
├── setup_wifi() - Conexão WiFi
├── callback() - Processamento de mensagens MQTT
├── reconnect() - Reconexão MQTT
├── setup() - Inicialização do sistema
└── loop() - Loop principal de execução
```

### Principais Funções

- **`setup_wifi()`**: Estabelece conexão com rede WiFi
- **`callback()`**: Processa comandos recebidos via MQTT
- **`reconnect()`**: Reconecta ao broker MQTT em caso de desconexão
- **Loop principal**: Executa medições e controle automático

### Protocolo de Comunicação

O sistema utiliza **MQTT (Message Queuing Telemetry Transport)** para comunicação:

- **Broker:** broker.hivemq.com (porta 1883)
- **Tópico de publicação:** `sensor/agua/nivel` - Envia medições de distância
- **Tópico de subscrição:** `atuador/agua/valvula` - Recebe comandos (1/0)
- **QoS:** 0 (Fire and forget)
- **Frequência de publicação:** A cada 500ms

## Descrição do Hardware

### Plataforma de Desenvolvimento

**ESP32 DevKit V1**
- Microcontrolador dual-core Xtensa LX6
- WiFi 802.11 b/g/n integrado
- Bluetooth 4.2 integrado
- 30 pinos GPIO programáveis
- Tensão de operação: 3.3V
- Tensão de entrada: 5V via USB

### Sensores

**HC-SR04 - Sensor Ultrassônico**
- Faixa de medição: 2cm a 400cm
- Precisão: ±3mm
- Ângulo de medição: 15°
- Tensão de operação: 5V
- Princípio: Eco ultrassônico (40kHz)

### Atuadores

**Módulo de Relé 5V (1 canal)**
- Tensão de controle: 5V
- Corrente máxima: 10A (250VAC), 10A (30VDC)
- Isolação óptica para proteção
- LED indicador de status

## Interfaces e Protocolos de Comunicação

### Protocolo MQTT

**Message Queuing Telemetry Transport (MQTT)**
- Protocolo de mensagens publish/subscribe
- Projetado para dispositivos IoT com recursos limitados
- Baixo consumo de largura de banda
- Confiável com diferentes níveis de QoS

### Estrutura de Comunicação

```
ESP32 ←→ WiFi ←→ Internet ←→ Broker MQTT (broker.hivemq.com)
                                    ↕
                              Cliente MQTT (App/Web)
```

### Interfaces Físicas

- **GPIO Digital:** Controle do relé (GPIO 19)
- **GPIO Digital:** Trigger do sensor (GPIO 5)
- **GPIO Digital:** Echo do sensor (GPIO 18)
- **Alimentação:** 5V e GND para componentes
- **Serial:** Comunicação para debug (115200 baud)

### Protocolo de Dados

**Formato das mensagens:**
- **Sensor → Broker:** `"XX.XX"` (distância em cm, formato string)
- **Broker → Atuador:** `"1"` (ligar) ou `"0"` (desligar)

**Exemplo de payload:**
```
Tópico: sensor/agua/nivel
Payload: "15.23"

Tópico: atuador/agua/valvula  
Payload: "1"
```

## Simulação e Testes

O projeto pode ser simulado na plataforma **Wokwi** utilizando os arquivos:
- `sketch.ino` - Código principal
- `diagram.json` - Diagrama de conexões
- `libraries.txt` - Dependências

**Link da simulação:** [Wokwi Project](https://wokwi.com/projects/427072965390909441)

## Arquivos do Projeto

- **`sketch.ino`** - Código fonte principal
- **`diagram.json`** - Configuração do circuito no Wokwi
- **`libraries.txt`** - Lista de bibliotecas necessárias
- **`README.md`** - Esta documentação

## Licença

Este projeto está sob a licença MIT. Consulte o arquivo LICENSE para mais detalhes.

## Autores

- **Matheus Ferraz Carvalho** - Desenvolvimento e implementação
- **Andre Luis de Oliveira** - Orientação e supervisão

**Universidade Presbiteriana Mackenzie**  
Faculdade de Computação e Informática  
São Paulo, SP - Brasil
