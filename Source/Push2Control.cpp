/*
  ==============================================================================

    Push2Control.cpp
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Push2Control.h"
#include <cctype>
#include "SynthGlobals.h"
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"

Push2Control::Push2Control()
: mDisplayModule(nullptr)
{
   NBase::Result result = Initialize();
   if (result.Succeeded())
   {
      ofLog() << "push 2 connected";
   }
   else
   {
      ofLog() << "push 2 failed to connect";
   }
}

Push2Control::~Push2Control()
{
}

void Push2Control::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   //UIBLOCK0();
   //ENDUIBLOCK(mWidth, mHeight);
   mWidth = 100;
   mHeight = 20;
}

void Push2Control::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}
 
void Push2Control::PostRender()
{
   RenderPush2Display();
}

void Push2Control::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Push2Control::SetUpFromSaveData()
{
}

void Push2Control::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

NBase::Result Push2Control::Initialize()
{
   mVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   
   const auto width = ableton::Push2DisplayBitmap::kWidth;
   const auto height = ableton::Push2DisplayBitmap::kHeight;
   mPixelRatio = 1;
   
   mFB = nvgluCreateFramebuffer(mVG, width*mPixelRatio, height*mPixelRatio, 0);
   if (mFB == NULL) {
      printf("Could not create FBO.\n");
   }
   
   // First we initialise the low level push2 object
   NBase::Result result = push2Display_.Init();
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init push2");

   // Then we initialise the juce to push bridge
   result = bridge_.Init(push2Display_);
   RETURN_IF_FAILED_MESSAGE(result, "Failed to init bridge");
   
   mPixels = new unsigned char[3 * (width * mPixelRatio) * (height * mPixelRatio)];
   
   mFontHandle = nvgCreateFont(mVG, ofToDataPath("frabk.ttf").c_str(), ofToDataPath("frabk.ttf").c_str());
   mFontHandleBold = nvgCreateFont(mVG, ofToDataPath("frabk_m.ttf").c_str(), ofToDataPath("frabk_m.ttf").c_str());

   return NBase::Result::NoError;
}

void Push2Control::DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio)
{
   int winWidth, winHeight;
   int fboWidth, fboHeight;

   if (fb == NULL)
      return;

   nvgImageSize(vg, fb->image, &fboWidth, &fboHeight);
   winWidth = (int)(fboWidth / pxRatio);
   winHeight = (int)(fboHeight / pxRatio);

   nvgluBindFramebuffer(fb);
   glViewport(0, 0, fboWidth, fboHeight);
   glClearColor(0, 0, 0, 0);
   glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
   
   nvgLineCap(vg, NVG_ROUND);
   nvgLineJoin(vg, NVG_ROUND);
   static float sSpacing = -.3f;
   nvgTextLetterSpacing(vg, sSpacing);
   
   ofSetColor(255,255,255);
   if (mDisplayModule != nullptr)
   {
      const float kSpacing = 121;
      
      vector<IUIControl*> controls = mDisplayModule->GetUIControls();
      int numDisplayableControls = 0;
      for (int i=0; i < controls.size(); ++i)
      {
         if (controls[i]->IsSimpleControl())
            ++numDisplayableControls;
      }
      
      //nvgFontSize(mVG, 16);
      //nvgText(mVG, 10, 10, mDisplayModule->Name(), nullptr);
      float x;
      float y;
      mDisplayModule->GetPosition(x, y, true);
      mDisplayModule->SetPosition(5, 15);
      float titleBarHeight;
      float highlight;
      mDisplayModule->DrawFrame(kSpacing * numDisplayableControls + 10, 45, false, titleBarHeight, highlight);
      mDisplayModule->SetPosition(x, y);
   
      nvgFontSize(mVG, 16);
      int displayIndex = 0;
      for (int i=0; i < controls.size(); ++i)
      {
         if (controls[i]->IsSimpleControl())
         {
            nvgFontFaceId(mVG, mFontHandleBold);
            nvgFontSize(mVG, 16);
            nvgText(mVG, kSpacing * displayIndex + 10, 15, controls[i]->Name(), nullptr);
            
            float x;
            float y;
            controls[i]->GetPosition(x, y, true);
            controls[i]->SetPosition(kSpacing * displayIndex + 10, 20);
            controls[i]->Draw();
            controls[i]->SetPosition(x, y);
            ++displayIndex;
         }
      }
   }

   nvgEndFrame(vg);
   
   glFinish();
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glReadPixels(0, 0, winWidth, winHeight, GL_RGB, GL_UNSIGNED_BYTE, mPixels);
   
   nvgluBindFramebuffer(NULL);
}

void Push2Control::RenderPush2Display()
{
   if (gHoveredModule != nullptr)
      mDisplayModule = gHoveredModule;
   
   auto mainVG = gNanoVG;
   gNanoVG = mVG;
   
   DrawToFramebuffer(mVG, mFB, gTime/300, mPixelRatio);
   
   gNanoVG = mainVG;

   // Tells the bridge we're done with drawing and the frame can be sent to the display
   bridge_.Flip(mPixels);
}