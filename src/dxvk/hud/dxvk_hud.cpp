#include <version.h>
#include "dxvk_hud.h"
#include <windows.h>
#include <time.h>

namespace dxvk::hud {
  float Hud::offset_x_float = 0;
  float Hud::offset_y_float = 0;
  bool show_hud = true;
  int lastPress;
  
  Hud::Hud(
    const Rc<DxvkDevice>& device,
    const HudConfig&      config)
  : m_config        (config),
    m_device        (device),
    m_uniformBuffer (createUniformBuffer()),
    m_renderer      (device),
    m_hudDeviceInfo (device),
    m_hudFramerate  (config.elements),
    m_hudStats      (config.elements) {
    // Set up constant state
    m_rsState.polygonMode       = VK_POLYGON_MODE_FILL;
    m_rsState.cullMode          = VK_CULL_MODE_BACK_BIT;
    m_rsState.frontFace         = VK_FRONT_FACE_CLOCKWISE;
    m_rsState.depthClipEnable   = VK_FALSE;
    m_rsState.depthBiasEnable   = VK_FALSE;
    m_rsState.sampleCount       = VK_SAMPLE_COUNT_1_BIT;

    m_blendMode.enableBlending  = VK_TRUE;
    m_blendMode.colorSrcFactor  = VK_BLEND_FACTOR_ONE;
    m_blendMode.colorDstFactor  = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    m_blendMode.colorBlendOp    = VK_BLEND_OP_ADD;
    m_blendMode.alphaSrcFactor  = VK_BLEND_FACTOR_ONE;
    m_blendMode.alphaDstFactor  = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    m_blendMode.alphaBlendOp    = VK_BLEND_OP_ADD;
    m_blendMode.writeMask       = VK_COLOR_COMPONENT_R_BIT
                                | VK_COLOR_COMPONENT_G_BIT
                                | VK_COLOR_COMPONENT_B_BIT
                                | VK_COLOR_COMPONENT_A_BIT;
  }
  
  
  Hud::~Hud() {
    
  }
  
  
  void Hud::update() {
    m_hudFramerate.update();
    m_hudStats.update(m_device);
  }
  
  
  void Hud::render(const Rc<DxvkContext>& ctx, VkExtent2D surfaceSize) {
    m_uniformData.surfaceSize = surfaceSize;
    
    this->updateUniformBuffer(ctx, m_uniformData);

    this->setupRendererState(ctx);
    this->renderHudElements(ctx);
  }
  
  
  Rc<Hud> Hud::createHud(const Rc<DxvkDevice>& device) {
    std::string hudElements = env::getEnvVar("DXVK_HUD");
    std::string offset_x = env::getEnvVar("DXVK_HUD_OFFSET_X");
    std::string offset_y = env::getEnvVar("DXVK_HUD_OFFSET_Y");
    
    if (!offset_x.empty())
      offset_x_float = stof(offset_x);
    
    if (!offset_y.empty())
      offset_y_float = stof(offset_y);

    if (hudElements.empty())
      hudElements = device->config().hud;

    HudConfig config(hudElements);

    return !config.elements.isClear()
      ? new Hud(device, config)
      : nullptr;
  }


  void Hud::setupRendererState(const Rc<DxvkContext>& ctx) {
    ctx->setRasterizerState(m_rsState);
    ctx->setBlendMode(0, m_blendMode);

    ctx->bindResourceBuffer(0,
      DxvkBufferSlice(m_uniformBuffer));

    m_renderer.beginFrame(ctx, m_uniformData.surfaceSize);
  }

  void Hud::renderHudElements(const Rc<DxvkContext>& ctx) {
    if(GetAsyncKeyState(VK_F12) & 0x8000)
    {
      if (GetTickCount() - lastPress > 500){
        lastPress = GetTickCount();
        if (show_hud){
          show_hud = false;
        } else {
          show_hud = true;
        }
      }
    } 
    
    HudPos position = { offset_x_float + 8.0f, offset_y_float + 24.0f };
    
    if (show_hud){
      if (m_config.elements.test(HudElement::DxvkVersion)) {
        m_renderer.drawText(ctx, 16.0f,
          { position.x, position.y },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          "DXVK " DXVK_VERSION);
        position.y += 24.0f;
      }

      if (m_config.elements.test(HudElement::DxvkClientApi)) {
        m_renderer.drawText(ctx, 16.0f,
          { position.x, position.y },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          m_device->clientApi());
        position.y += 24.0f;
      }

      if (m_config.elements.test(HudElement::DeviceInfo)) {
        position = m_hudDeviceInfo.render(
          ctx, m_renderer, position);
      }
      position = m_hudFramerate.render(ctx, m_renderer, position);
      position = m_hudStats    .render(ctx, m_renderer, position);
    }
  }
  
  
  void Hud::updateUniformBuffer(const Rc<DxvkContext>& ctx, const HudUniformData& data) {
    auto slice = m_uniformBuffer->allocSlice();
    std::memcpy(slice.mapPtr, &data, sizeof(data));

    ctx->invalidateBuffer(m_uniformBuffer, slice);
  }


  Rc<DxvkBuffer> Hud::createUniformBuffer() {
    DxvkBufferCreateInfo info;
    info.size           = sizeof(HudUniformData);
    info.usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    info.stages         = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
                        | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    info.access         = VK_ACCESS_UNIFORM_READ_BIT;
    
    return m_device->createBuffer(info,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
  
}
