#pragma once

#include <simd/simd.h>

#include "AAPLMathUtilities.h"
#include "mesh.hpp"
#include "model.hpp"
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/CAMetalLayer.hpp>

#include <iostream>

class MTLEngine {
public:
  CA::MetalDrawable *run(float3 position, float pitch, float yaw,
                         matrix_float4x4 modelMatrix);
  void init(int width, int height);
  void cleanup();

private:
  void initDevice();

  void createCommandQueue();
  void loadMeshes();
  void createDefaultLibrary();
  void createRenderPipeline();
  void createDepthAndMSAATextures();
  void createRenderPassDescriptor();

  // Upon resizing, update Depth and MSAA Textures.
  void updateRenderPassDescriptor();

  void draw();
  void sendRenderCommand(float3 position, float pitch, float yaw,
                         matrix_float4x4 modelMatrix);
  void encodeRenderCommand(MTL::RenderCommandEncoder *renderCommandEncoder,
                           float3 position, float pitch, float yaw,
                           matrix_float4x4 modelMatrix);

  MTL::Device *metalDevice;
  CA::MetalLayer *metalLayer;
  CA::MetalDrawable *metalDrawable;
  bool windowResizeFlag = false;
  int newWidth, newHeight;
  float3 up = make_float3(0.0f, -1.0f, 0.0f);

  NS::Error *error = nullptr;
  MTL::Library *metalDefaultLibrary;
  MTL::CommandQueue *metalCommandQueue;
  MTL::CommandBuffer *metalCommandBuffer;
  MTL::RenderPipelineState *metalRenderPSO;
  MTL::RenderPassDescriptor *renderPassDescriptor;
  Mesh *mesh;
  MTL::DepthStencilState *depthStencilState;
  MTL::Texture *msaaRenderTargetTexture;
  MTL::Texture *depthTexture;
  Model *model;
  int sampleCount = 4;
};
