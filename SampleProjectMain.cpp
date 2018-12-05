//
//  SampleProjectMain.cpp
//  SampleProjectTarget
//
//  Created by Gilbert Gwizdała on 03/12/2018.
//

#include "SampleProjectMain.hpp"

#include <string>
#include <fstream>
#include <sstream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

using namespace Urho3D;

void SampleProjectMain::Start() {
    
    Application::Start();
    
    Setup();
    
    CreateScene();
}

void SampleProjectMain::Setup() { 
    engineParameters_["FullScreen"]=false;
    engineParameters_["WindowWidth"]=1280;
    engineParameters_["WindowHeight"]=720;
    engineParameters_["WindowResizable"]=true;
}

template<typename T> void set_model(T* model, Urho3D::ResourceCache* cache, std::string model_name)
{
    std::string filename_model=model_name;
    model->SetModel(cache->GetResource<Urho3D::Model>(Urho3D::String(filename_model.append(".mdl").c_str())));
    std::string filename_txt=model_name;
    filename_txt.append(".txt");
    std::ifstream file(filename_txt.c_str());
    std::string line;
    if(file.is_open())
        for(int i=0;getline(file,line);i++)
            model->SetMaterial(i,cache->GetResource<Urho3D::Material>(Urho3D::String(line.c_str())));
}

void SampleProjectMain::CreateScene() { 
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    this->scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    this->cameraNode_ = this->scene_->CreateChild("Camera");
    this->camera_ = this->cameraNode_->CreateComponent<Camera>();
    this->camera_->SetFarClip(600);
    this->camera_->SetNearClip(0.1);
    this->camera_->SetFov(75);
    auto* listener=cameraNode_->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(listener);
    GetSubsystem<Audio>()->SetMasterGain(SOUND_MUSIC, 0.3);

    auto* renderer = GetSubsystem<Renderer>();
    SharedPtr<Viewport> viewport(new Viewport(this->context_, this->scene_, this->cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
    renderer->SetShadowMapSize(1024);

    auto* effectRenderPath=viewport->GetRenderPath();
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR_stronger.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));

    auto* zoneNode = this->scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-50000.0f, 50000.0f));
    zone->SetFogStart(500.0f);
    zone->SetFogEnd(600.0f);
    zone->SetFogColor(Color(1, 1, 1));
    zone->SetAmbientColor(Color(0.1, 0.1, 0.1));

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(SampleProjectMain, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SampleProjectMain, HandleUpdate));

    this->cameraNode_->SetPosition(Vector3(0, 0, 0));
    this->cameraNode_->SetDirection(Vector3::FORWARD);

    // create a transparent window with some text to display things like help and FPS
    {
        this->window = new Window(context_);
        GetSubsystem<UI>()->GetRoot()->AddChild(this->window);
        this->window->SetStyle("Window");
        this->window->SetSize(600, 70);
        this->window->SetColor(Color(.0,.15,.3,.5));
        this->window->SetAlignment(HA_LEFT, VA_TOP);

        this->window_text = new Text(context_);
        this->window_text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 14);
        this->window_text->SetColor(Color(.8, .85, .9));
        this->window_text->SetAlignment(HA_LEFT, VA_TOP);
        this->window->AddChild(window_text);
    }

    // a rotating planet
    {
        this->node_rotating_planet = this->scene_->CreateChild("Planet");
        this->node_rotating_planet->SetPosition(Vector3(-4, 1.6, 6));
        this->node_rotating_planet->Scale(2);
        auto* boxObject = node_rotating_planet->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/planet.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/planet_dsn.xml"));
        boxObject->SetCastShadows(true);
    }

    // skybox
    {
        this->skyNode = this->scene_->CreateChild("Sky");
        this->skyNode->SetScale(1500.0f);
        auto* skybox = skyNode->CreateComponent<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("Data/Models/Box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("Data/Materials/Skybox.xml"));
    }
    
    // a torch with a light, sound and particle effects
    {
        this->node_torch = this->scene_->CreateChild("Torch");
        Vector3 pos(Vector3(3,-0.3,6));
        this->node_torch->SetPosition(pos);

        auto* boxObject = node_torch->CreateComponent<StaticModel>();
        set_model(boxObject, cache, "Data/Models/torch");
        boxObject->SetCastShadows(true);
        boxObject->SetOccludee(true);
        boxObject->SetShadowDistance(200);
        boxObject->SetDrawDistance(200);

        auto* lightNode = this->node_torch->CreateChild();
        lightNode->Translate(Vector3(0, 2, 0));
        auto* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(1.0,0.6,0.3,1.0));
        light->SetCastShadows(true);
        light->SetShadowDistance(200);
        light->SetDrawDistance(200);

        auto* n_particle = this->node_torch->CreateChild();
        n_particle->Translate(Vector3(0, 1.6, 0));
        auto* emitter=n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/torch_fire.xml"));
        emitter = n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/torch_smoke.xml"));

        auto* sound_torch = cache->GetResource<Sound>("Sounds/torch.ogg");
        sound_torch->SetLooped(true);
        auto* sound_torch_source = n_particle->CreateComponent<SoundSource3D>();
        sound_torch_source->SetNearDistance(1);
        sound_torch_source->SetFarDistance(50);
        sound_torch_source->SetSoundType(SOUND_EFFECT);
        sound_torch_source->Play(sound_torch);
    }

    // sun
    {
        this->lightNode = this->scene_->CreateChild("Light");
        auto* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00025f, 0.7f));
        light->SetShadowCascade(CascadeParameters(4.0f, 16.0f, 64.0f, 128.0f, 0.8f));
        light->SetColor(Color(1.4, 0.9, 0.8, 1));
        this->lightNode->SetDirection(Vector3::FORWARD);
        this->lightNode->Yaw(-150);   // horizontal
        this->lightNode->Pitch(30);   // vertical
        this->lightNode->Translate(Vector3(0, 0, -20000));

        auto* billboardObject = this->lightNode->CreateComponent<BillboardSet>();
        billboardObject->SetNumBillboards(1);
        billboardObject->SetMaterial(cache->GetResource<Material>("Materials/sun.xml"));
        billboardObject->SetSorted(true);
        Billboard* bb = billboardObject->GetBillboard(0);
        bb->size_=Vector2(10000, 10000);
        bb->rotation_ = Random()*360.0f;
        bb->enabled_ = true;
        billboardObject->Commit();
    }

    {
        auto* terrainNode = this->scene_->CreateChild("Terrain");
        terrainNode->SetPosition(Vector3(3.0f, -0.4f));
        this->terrain = terrainNode->CreateComponent<Terrain>();
        this->terrain->SetPatchSize(128);
        this->terrain->SetSpacing(Vector3(2, 0.5, 2));
        this->terrain->SetSmoothing(true);
        this->terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
        this->terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
        this->terrain->SetCastShadows(true);
        this->terrain->SetOccluder(true);
    }
}

void SampleProjectMain::HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) { 
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();
    
    if(key == KEY_TAB) {
        GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
        GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
    }
    else if(key == KEY_ESCAPE)
        this->engine_->Exit();
    else if(key==KEY_G)
        this->window->SetVisible(!window->IsVisible());
    else if(key==KEY_T)
        this->camera_->SetFillMode(camera_->GetFillMode() == FILL_WIREFRAME ? FILL_SOLID : FILL_WIREFRAME);
}

void SampleProjectMain::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) { 
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    
    std::string str="WASD, mouse and shift to move. T to toggle fill mode,\nG to toggle GUI, Tab to toggle mouse mode, Esc to quit.\n";
    {
        std::ostringstream ss;
        ss<<1/timeStep;
        std::string s(ss.str());
        str.append(s.substr(0,6));
    }
    str.append(" FPS ");
    String s(str.c_str(),str.size());
    this->window_text->SetText(s);
    
    this->node_rotating_planet->Rotate(Quaternion(0, -22*timeStep, 0));
    
    // Movement speed as world units per second
    float MOVE_SPEED=10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;
    
    // camera movement
    Input* input = GetSubsystem<Input>();
    if(input->GetQualifierDown(Qualifier(1)))  // 1 is shift, 2 is ctrl, 4 is alt
        MOVE_SPEED*=10;
    if(input->GetKeyDown(KEY_W))
        cameraNode_->Translate(Vector3(0, 0, 1)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown(KEY_S))
        cameraNode_->Translate(Vector3(0, 0, -1)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown(KEY_A))
        cameraNode_->Translate(Vector3(-1, 0, 0)*MOVE_SPEED*timeStep);
    if(input->GetKeyDown(KEY_D))
        cameraNode_->Translate(Vector3( 1, 0, 0)*MOVE_SPEED*timeStep);
    
    if(!GetSubsystem<Input>()->IsMouseVisible())
    {
        IntVector2 mouseMove=input->GetMouseMove();
        if( mouseMove.x_ > -2000000000 && mouseMove.y_ > -2000000000 )
        {
            static float yaw_=0;
            static float pitch_=0;
            yaw_ += MOUSE_SENSITIVITY*mouseMove.x_;
            pitch_ += MOUSE_SENSITIVITY*mouseMove.y_;
            pitch_ = Clamp(pitch_, -90.0f, 90.0f);
            // Reset rotation and set yaw and pitch again
            this->cameraNode_->SetDirection(Vector3::FORWARD);
            this->cameraNode_->Yaw(yaw_);
            this->cameraNode_->Pitch(pitch_);
        }
    }

}




