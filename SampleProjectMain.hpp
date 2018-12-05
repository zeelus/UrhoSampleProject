//
//  SampleProjectMain.hpp
//  SampleProjectTarget
//
//  Created by Gilbert Gwizdała on 03/12/2018.
//

#ifndef SampleProjectMain_hpp
#define SampleProjectMain_hpp

#include <Urho3D/Engine/Application.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Scene/Scene.h>


namespace Urho3D
{
    
    class Node;
    class Scene;
    class String;
    
}

using namespace Urho3D;

class SampleProjectMain : public Application {
    
    SharedPtr<Scene> scene_;
    Node* cameraNode_;
    SharedPtr<Urho3D::Node> node_rotating_planet;
    Urho3D::Text* window_text;
    SharedPtr<Urho3D::Window> window;
    Urho3D::Terrain* terrain;
    Urho3D::Camera* camera_;
    SharedPtr<Node> skyNode;
    SharedPtr<Node> node_torch;
    SharedPtr<Node> lightNode;
    
public:

    SampleProjectMain(Context * context) : Application(context) {}
    
    virtual void Start();
    
private:
    
    void Setup();
    
    void CreateScene();
    
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    
};

#endif /* SampleProjectMain_hpp */
