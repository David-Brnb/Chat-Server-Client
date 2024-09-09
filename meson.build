project('chat-app', 'cpp',
  version: '0.1',
  default_options: ['cpp_std=c++14'])

# Define la fuente del proyecto
src = ['src/Server.cpp', 'src/User.cpp', 'src/main.cpp']

# Agregar la dependencia de nlohmann/json
nlohmann_json = dependency('nlohmann_json', version: '>=3.9.1')

# Crear el ejecutable
executable('chat-server', src, dependencies: [nlohmann_json])
