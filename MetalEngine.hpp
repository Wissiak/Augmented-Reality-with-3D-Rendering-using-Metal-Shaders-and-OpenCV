#pragma once

#include <simd/simd.h>

#include "mesh.hpp"
#include "model.hpp"
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include "AAPLMathUtilities.h"

#include <iostream>

class MTLEngine {
public:
    void init(int width, int height);
    CA::MetalDrawable* run(float3 position, float pitch, float yaw);
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
    void sendRenderCommand(float3 position, float pitch, float yaw);
    void encodeRenderCommand(MTL::RenderCommandEncoder* renderCommandEncoder, float3 position, float pitch, float yaw);
    
    MTL::Device* metalDevice;
    CA::MetalLayer* metalLayer;
    CA::MetalDrawable* metalDrawable;
    bool windowResizeFlag = false;
    int newWidth, newHeight;
    
    NS::Error *error = nullptr;
    MTL::Library* metalDefaultLibrary;
    MTL::CommandQueue* metalCommandQueue;
    MTL::CommandBuffer* metalCommandBuffer;
    MTL::RenderPipelineState* metalRenderPSO;
    MTL::RenderPassDescriptor* renderPassDescriptor;
    Mesh* mesh;
    MTL::DepthStencilState* depthStencilState;
    MTL::Texture* msaaRenderTargetTexture;
    MTL::Texture* depthTexture;
    Model* model;
    int sampleCount = 4;    
};
