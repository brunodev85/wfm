#ifndef STRINGS_PT_H
#define STRINGS_PT_H

#include "resource.h"

static inline void loadStrings_pt() {
    lc_str.app_name = APP_NAME;
    lc_str.app_version = L"Versão " APP_VERSION;
    lc_str.app_dev_name = L"por " APP_DEV_NAME;
    lc_str.application = L"Aplicação";
    lc_str.shortcut = L"Atalho";
    lc_str.file = L"Arquivo";
    lc_str.folder = L"Pasta";
    lc_str.local_drive = L"Unidade Local";
    lc_str.cd_drive = L"Unidade de CD";
    lc_str.computer = L"Computador";
    lc_str.desktop = L"Desktop";
    lc_str.documents = L"Documentos";
    lc_str.exit = L"Sair";
    lc_str.edit = L"Editar";
    lc_str.cut = L"Recortar";
    lc_str.copy = L"Copiar";
    lc_str.paste = L"Colar";
    lc_str.paste_shortcut = L"Colar Atalho";
    lc_str.select_all = L"Selecionar Tudo";
    lc_str.view = L"Visualizar";
    lc_str.large_icons = L"Ícones Grandes";
    lc_str.small_icons = L"Ícones Pequenos";
    lc_str.list = L"Lista";
    lc_str.details = L"Detalhes";
    lc_str.help = L"Ajuda";
    lc_str.about = L"Sobre";
    lc_str.ok = L"OK";
    lc_str.cancel = L"Cancelar";
    lc_str.loading = L"Carregando...";
    lc_str.open = L"Abrir";
    lc_str.create_shortcut = L"Criar Atalho";
    lc_str.delete = L"Excluir";
    lc_str.rename = L"Renomear";
    lc_str.new_folder = L"Nova Pasta";
    lc_str.new_file = L"Novo Arquivo";
    lc_str.items = L"Itens";
    lc_str.load_iso_image = L"Carregar Imagem ISO";
    lc_str.unload_iso_image = L"Descarregar Imagem ISO";
    lc_str.no_media = L"Sem mídia";
    lc_str.alert = L"Alerta";
    lc_str.enter_folder_name = L"Digite o nome da pasta:";
    lc_str.enter_file_name = L"Digite o nome do arquivo:";
    lc_str.enter_new_name = L"Digite o novo nome:";
    lc_str.name = L"Nome";
    lc_str.type = L"Tipo";
    lc_str.size = L"Tamanho";
    lc_str.date = L"Data";
    lc_str.path = L"Caminho";
    lc_str.deleting_files = L"Excluindo arquivos";
    lc_str.copying_files = L"Copiando arquivos";
    lc_str.moving_files = L"Movendo arquivos";
    lc_str.extracting_files = L"Extraindo arquivos";
    lc_str.confirm_delete = L"Confirmar Excluir";
    lc_str.confirm_exit = L"Confirmar Saída";
    lc_str.search = L"Pesquisar";
    lc_str.up = L"Acima";

    lc_str.fmt_file = L"Arquivo %ls";
    
    lc_str.msg_invalid_iso_image_file = L"Arquivo de Imagem ISO inválido!";
    lc_str.msg_deleting_files = L"Excluindo arquivos, aguarde...";
    lc_str.msg_copying_files = L"Copiando arquivos, aguarde...";
    lc_str.msg_moving_files = L"Movendo arquivos, aguarde...";
    lc_str.msg_extracting_files = L"Extraindo arquivos, aguarde...";
    lc_str.msg_cancel_file_operation = L"Você quer cancelar a operação?";
    lc_str.msg_confirm_delete_item = L"Tem certeza de que deseja excluir \"%ls\"?";
    lc_str.msg_confirm_delete_multiple_items = L"Tem certeza de que deseja excluir %d itens?";
    lc_str.msg_confirm_exit_app = L"Tem certeza de que deseja sair?";
}

#endif