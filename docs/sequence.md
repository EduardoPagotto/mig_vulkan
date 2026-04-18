# Como funciona a API!

## Sequencia de Inicialização do VulKan:

1- Inicializa SDL_VIDEO

2- Criar window

3- Validar extençoes do SDL com Vulkan
  3.1- executar SDL_Vulkan_GetInstanceExtensions para pegar totais de extencoes SDL
  3.2- executar vkEnumerateInstanceExtensionProperties para pegar lista de extencoes Vulkan
  3.3- Percorre (3.1) e valida exitencia em cada (3.2)

4- Criar Instancia
  4.1- criar struct VkApplicationInfo
  4.2- Criar struct VkInstanceCreateInfo com (3.1) e (4.1)

5- Pegar o PhysicalDevice
  5.1 - executar vkEnumeratePhysicalDevices para pegar lista de PhysicalDevice
  5.2 - executar vkGetPhysicalDeviceQueueFamilyProperties cada item da lista de 5.1
  5.3 - criar QueueFamilyIndices e validar se tem flags compativeis com cada 5.2
  5.4 - usar o primeiro device no struct mainDevice.physicalDevice

6- Criar o LogicalDevice
  6.1 - pegar indices com struct QueueFamilyIndices com parametro (5.4)
  6.2 - criar struct VkDeviceQueueCreateInfo usando indice de (6.1)
  6.3 - criar struct VkDeviceCreateInfo usando (6.2)
  6.4 - criar struct VkPhysicalDeviceFeatures
  6.5 - usar 6.4 em 6.3
  6.6 - executar vkCreateDevice passando 5.4, 6.2, resultado em mainDevice.logicalDevice
  6.7 - executar vkGetDeviceQueue pasando 5.3 e 6.6 resultado em VkQueue

## Swapchain

7-
