#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "MetalEngine.hpp"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>

/*
MTLEngine() {

    _mDevice = device;

    NS::Error *error = nullptr;

    // Load the shader files with a .metal file extension in the project
    NS::String* filePath = NS::String::string("../add.metallib",
NS::UTF8StringEncoding); MTL::Library *defaultLibrary =
_mDevice->newLibrary(filePath, &error);

    assert(defaultLibrary != nullptr);


    auto str = NS::String::string("add_arrays", NS::ASCIIStringEncoding);
    MTL::Function *addFunction = defaultLibrary->newFunction(str);
    defaultLibrary->release();

    assert(addFunction != nullptr);

    // Create a compute pipeline state object.
    _mAddFunctionPSO = _mDevice->newComputePipelineState(addFunction, &error);
    addFunction->release();

    assert(_mAddFunctionPSO != nullptr);

    _mCommandQueue = _mDevice->newCommandQueue();

    assert(_mCommandQueue != nullptr);

    // Allocate three buffers to hold our initial data and the result.
    _mBufferA = _mDevice->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    _mBufferB = _mDevice->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    _mBufferResult = _mDevice->newBuffer(bufferSize,
MTL::ResourceStorageModeShared);

    prepareData();
}*/

void MTLEngine::init() {
  initDevice();

  metalLayer = CA::MetalLayer::layer();
  metalLayer->setDevice(metalDevice);
  metalLayer->setDrawableSize(CGSizeMake(900, 800));

  createCommandQueue();
  createDefaultLibrary();
  loadMeshes();
  createRenderPipeline();
  createDepthAndMSAATextures();
  createRenderPassDescriptor();
}

void MTLEngine::initDevice() { metalDevice = MTL::CreateSystemDefaultDevice(); }
void MTLEngine::createCommandQueue() {
  metalCommandQueue = metalDevice->newCommandQueue();
  assert(metalCommandQueue != nullptr);
}
void MTLEngine::createDefaultLibrary() {
  NS::String *filePath =
      NS::String::string("../model.metallib", NS::UTF8StringEncoding);
  metalDefaultLibrary = metalDevice->newLibrary(filePath, &error);

  assert(metalDefaultLibrary != nullptr);
}

void MTLEngine::loadMeshes() {
  model = new Model("../assets/tutorial.obj", metalDevice);

  std::cout << "Mesh Count: " << model->meshes.size() << std::endl;
}

void MTLEngine::createRenderPipeline() {
  MTL::Function *vertexShader = metalDefaultLibrary->newFunction(
      NS::String::string("vertexShader", NS::ASCIIStringEncoding));
  assert(vertexShader);
  MTL::Function *fragmentShader = metalDefaultLibrary->newFunction(
      NS::String::string("fragmentShader", NS::ASCIIStringEncoding));
  assert(fragmentShader);

  MTL::RenderPipelineDescriptor *renderPipelineDescriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();
  renderPipelineDescriptor->setVertexFunction(vertexShader);
  renderPipelineDescriptor->setFragmentFunction(fragmentShader);
  assert(renderPipelineDescriptor);
  MTL::PixelFormat pixelFormat = (MTL::PixelFormat)metalLayer->pixelFormat();
  renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(
      pixelFormat);
  renderPipelineDescriptor->setSampleCount(4);
  renderPipelineDescriptor->setLabel(
      NS::String::string("Model Render Pipeline", NS::ASCIIStringEncoding));
  renderPipelineDescriptor->setDepthAttachmentPixelFormat(
      MTL::PixelFormatDepth32Float);
  renderPipelineDescriptor->setTessellationOutputWindingOrder(
      MTL::WindingCounterClockwise);

  NS::Error *error;
  metalRenderPSO =
      metalDevice->newRenderPipelineState(renderPipelineDescriptor, &error);

  if (metalRenderPSO == nil) {
    std::cout << "Error creating render pipeline state: " << error << std::endl;
    std::exit(0);
  }

  MTL::DepthStencilDescriptor *depthStencilDescriptor =
      MTL::DepthStencilDescriptor::alloc()->init();
  depthStencilDescriptor->setDepthCompareFunction(
      MTL::CompareFunctionLessEqual);
  depthStencilDescriptor->setDepthWriteEnabled(true);
  depthStencilState = metalDevice->newDepthStencilState(depthStencilDescriptor);

  depthStencilDescriptor->release();
  renderPipelineDescriptor->release();
  vertexShader->release();
  fragmentShader->release();
}

CA::MetalDrawable* MTLEngine::run() {
  return metalLayer->nextDrawable();
}

void MTLEngine::createDepthAndMSAATextures() {
  MTL::TextureDescriptor *msaaTextureDescriptor =
      MTL::TextureDescriptor::alloc()->init();
  msaaTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
  msaaTextureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
  msaaTextureDescriptor->setWidth(metalLayer->drawableSize().width);
  msaaTextureDescriptor->setHeight(metalLayer->drawableSize().height);
  msaaTextureDescriptor->setSampleCount(sampleCount);
  msaaTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);

  msaaRenderTargetTexture = metalDevice->newTexture(msaaTextureDescriptor);

  MTL::TextureDescriptor *depthTextureDescriptor =
      MTL::TextureDescriptor::alloc()->init();
  depthTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
  depthTextureDescriptor->setPixelFormat(MTL::PixelFormatDepth32Float);
  depthTextureDescriptor->setWidth(metalLayer->drawableSize().width);
  depthTextureDescriptor->setHeight(metalLayer->drawableSize().height);
  depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
  depthTextureDescriptor->setSampleCount(sampleCount);

  depthTexture = metalDevice->newTexture(depthTextureDescriptor);

  msaaTextureDescriptor->release();
  depthTextureDescriptor->release();
}

void MTLEngine::createRenderPassDescriptor() {
  renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();

  MTL::RenderPassColorAttachmentDescriptor *colorAttachment =
      renderPassDescriptor->colorAttachments()->object(0);
  MTL::RenderPassDepthAttachmentDescriptor *depthAttachment =
      renderPassDescriptor->depthAttachment();

  colorAttachment->setTexture(msaaRenderTargetTexture);
  colorAttachment->setResolveTexture(metalDrawable->texture());
  colorAttachment->setLoadAction(MTL::LoadActionClear);
  colorAttachment->setClearColor(
      MTL::ClearColor(41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0));
  colorAttachment->setStoreAction(MTL::StoreActionMultisampleResolve);

  depthAttachment->setTexture(depthTexture);
  depthAttachment->setLoadAction(MTL::LoadActionClear);
  depthAttachment->setStoreAction(MTL::StoreActionDontCare);
  depthAttachment->setClearDepth(1.0);
}

void MTLEngine::cleanup() {
  delete model;
  depthTexture->release();
  renderPassDescriptor->release();
  metalDevice->release();
}