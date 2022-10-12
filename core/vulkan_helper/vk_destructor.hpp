#ifndef VK_DESTRUCTOR_HPP
#define VK_DESTRUCTOR_HPP

#include "vk_instance.hpp"
namespace vkh {

void DestroyInstance(Instance instance);

void DestroyDevice(class Device device);

void DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                VkAllocationCallbacks* allocation_callbacks);
}  // namespace vkh
#endif  //VK_DESTRUCTOR_HPP
