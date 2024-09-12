project('chat-app', 'cpp', version : '1.0', default_options : ['cpp_std=c++17'])

# Ruta a los directorios del código fuente
server_src = files(
  'src/server/Server.cpp',
  'src/server/User.cpp',
  'src/server/main_server.cpp'
)

client_src = files(
  'src/client/Client.cpp',
  'src/client/main_client.cpp'
)

# Incluye la librería json si la usas
nlohmann_json = dependency('nlohmann_json', required : true)

# Definimos el directorio de los headers compartidos
inc_dir = include_directories('src/include')

# Definimos el ejecutable del servidor
executable('server', server_src,
           install : true,
           dependencies : [nlohmann_json],
           include_directories: inc_dir)

# Definimos el ejecutable del cliente
executable('client', client_src,
           install : true,
           dependencies : [nlohmann_json],
           include_directories: inc_dir)
