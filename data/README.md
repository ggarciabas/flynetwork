
# Estrutura de pasta dos resultados
- scenario
-   custo_
-       client
-       course-changed
-       dhcp
-       etapa
-       python 
-       uav_depletion
-       uav_energy_threshold
-       uav_hover
-       uav_move
-       uav_recharged
-       uav_remaining_energy
-       uav_stop

# Scripts para processamento pós-resultados
- run_pos_processing
    - general_graphic
    - uav_removed_energy
    - connected_cli

- run_slide_processing
    - process_slide

- run_exaustive
    - parametros:
        - debug
        - nome da pasta do cenario
        - etapa a ser avaliada
    - objetivo comparar com o exaustivo, sequencial e aleatorio todos os custos a estapa especifica.


# Descricao dos arquivos
Uav Energy [utilizado para armazenar informacoes da energia (em Joules) atual do UAv bem como o total inicial, para fazer comparacoes entre os custos]
> UAV_ID, ATUAL_ENERGY, INITIAL_ENERGY

Course Changed [energia necessária para voltar a central após chegar ao seu destino]
> TIME, X, Y, ENERGY_TO_CENTRAL

Cost Energy [custo para calculo de distancia e para flutuar]
> ENERGY_COST, HOVER_COST

Uav Depletion - [para identificar se havia bateria suficiente para ele retornar]
> TIME, REMAINIG ENERGY-ENERGY_TO_CENTRAL

Uav Remaining Energy (Uav Battery All) [porcentagem restante considerando todos os custos: mover e flutuar]
> TIME, REMAINING_ENERGY_%

Uav Moving [consumo energético em porcentagem para que o UAV se movimente]
> TIME, ENERGY_TO_MOVE_%

Uav Moving Acumulated [consumo energético em porcentagem acumulado para que o UAV se movimente]
> TIME, ENERGY_TO_MOVE_ACUMULATED_%

Uav Client [consumo energetico em porcentagem dos clientes conectados ao UAV]
> TIME, ENERGY_CLIENT_%

Uav Client [consumo energetico em porcentagem acumuladp dos clientes conectados ao UAV]
> TIME, ENERGY_CLIENT_ACUMULATED_%

Uav Hover [consumo energético em porcentagem para que o UAV flutue]
> TIME, ENERGY_HOVE_%

Uav Hover Acumulated [consumo energético em porcentagem acumulado para que o UAV flutue]
> TIME, ENERGY_TO_HOVER_ACUMULATED_%

Cenário Configuration [parametros utilizados na simulação do DA Python]

Cenário Input [informações enviadas no NS3 para o DA Python]
> ENVIRONMENT
> LOCATION_CONTAINER_SIZE
> CENTRAL_POSITION
> LOGIN, CLIENT_X, CLIENT_Y

Cenário Output [informações que o NS3 interpreta vindas do DA Python]
> UAV_X, UAV_Y
> LIST_OF_LOGIN_CLIENTS_IN_THAT_POSITION

Custo 1 [matriz que contempla o custo 1]
> LIST_OF_UAV_IDS
> LIST_OF_LOCATION_IDS
> MATRIX_OF_COST_OF_EACH_UAV_AND_LOCATION

Custo 2 [matriz que contempla o custo 2]
> LIST_OF_UAV_IDS
> LIST_OF_LOCATION_IDS
> MATRIX_OF_COST_OF_EACH_UAV_AND_LOCATION

Custo 3 [matriz que contempla o custo 3]
> LIST_OF_UAV_IDS
> LIST_OF_LOCATION_IDS
> MATRIX_OF_COST_OF_EACH_UAV_AND_LOCATION

Client [para identificar a posicao dos clientes que o servidor conhece e a posicao real no cenario]
> CLIENT_SERVER_X, CLIENT_SERVER_Y, CLIENT_SERVER_ID, [...]
> CLIENT_REAL_X, CLIENT_REAL_Y, CLIENT_REAL_ID, [...]

Localização e clientes [ para se ter uma relação do total de clientes e o consumo total de cada localização]
> LOC_ID, TOTAL_CLI, TOTAL_CONSUMPTION
