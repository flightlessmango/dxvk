#pragma once

#include <chrono>

#include "dxvk_hud_config.h"
#include "dxvk_hud_renderer.h"

namespace dxvk::hud {
  
  /**
   * \brief FPS display for the HUD
   * 
   * Displays the current frames per second.
   */
  class HudFps {
    using Clock     = std::chrono::high_resolution_clock;
    using TimeDiff  = std::chrono::microseconds;
    using TimePoint = typename Clock::time_point;
    
    constexpr static uint32_t NumDataPoints  = 300;
    constexpr static int64_t  UpdateInterval = 500'000;
  public:
    
    HudFps(HudElements elements);
    ~HudFps();
    
    void update();
    
    HudPos render(
      const Rc<DxvkContext>&  context,
            HudRenderer&      renderer,
            HudPos            position);
    
  private:
    
    const HudElements m_elements;
    
    std::string m_fpsString;
    std::string m_cpuUtilizationString;
    bool mango_logging = false;
    time_t lastPress = time(0);
    char const* logging = getenv("DXVK_LOG_TO_FILE");
    int64_t fps;
    
    TimePoint m_prevFpsUpdate;
    TimePoint m_prevFtgUpdate;
    TimePoint m_prevF2Press;
    TimeDiff  elapsedF2;
    int64_t   m_frameCount = 0;
    
    std::array<float, NumDataPoints>  m_dataPoints  = {};
    uint32_t                          m_dataPointId = 0;

    HudPos renderGpuText(
      const Rc<DxvkContext>&  context,
      HudRenderer&      renderer,
      HudPos            position);
      
    HudPos renderCpuText(
      const Rc<DxvkContext>&  context,
            HudRenderer&      renderer,
            HudPos            position);
    
    HudPos renderFpsText(
      const Rc<DxvkContext>&  context,
            HudRenderer&      renderer,
            HudPos            position);
    
    
    HudPos renderFrametimeGraph(
      const Rc<DxvkContext>&  context,
            HudRenderer&      renderer,
            HudPos            position);
            
    HudPos renderLogging(
      const Rc<DxvkContext>&  context,
            HudRenderer&      renderer,
            HudPos            position);
    
  };
  
}