/*
 * GDevelop Core
 * Copyright 2008-2016 Florian Rival (Florian.Rival@gmail.com). All rights
 * reserved. This project is released under the MIT License.
 */
#include "GDCore/Project/Object.h"
#include "GDCore/Extensions/Platform.h"
#include "GDCore/Project/Behavior.h"
#include "GDCore/Project/Layout.h"
#include "GDCore/Project/Project.h"
#include "GDCore/Serialization/SerializerElement.h"
#if defined(GD_IDE_ONLY)
#include "GDCore/IDE/Dialogs/PropertyDescriptor.h"
#endif

namespace gd {

Object::~Object() {}

Object::Object(const gd::String& name_) : name(name_) {}

void Object::Init(const gd::Object& object) {
  name = object.name;
  type = object.type;
  objectVariables = object.objectVariables;

  behaviors.clear();
  for (auto& it : object.behaviors)
    behaviors[it.first] = gd::make_unique<gd::BehaviorContent>(*it.second);
}

std::vector<gd::String> Object::GetAllBehaviorNames() const {
  std::vector<gd::String> allNameIdentifiers;

  for (auto& it : behaviors)
    allNameIdentifiers.push_back(it.first);

  return allNameIdentifiers;
}

void Object::RemoveBehavior(const gd::String& name) { behaviors.erase(name); }

bool Object::RenameBehavior(const gd::String& name, const gd::String& newName) {
  if (behaviors.find(name) == behaviors.end() ||
      behaviors.find(newName) != behaviors.end())
    return false;

  std::unique_ptr<BehaviorContent> aut = std::move(behaviors.find(name)->second);
  behaviors.erase(name);
  behaviors[newName] = std::move(aut);
  behaviors[newName]->SetName(newName);

  return true;
}

gd::BehaviorContent& Object::GetBehavior(const gd::String& name) {
  return *behaviors.find(name)->second;
}

const gd::BehaviorContent& Object::GetBehavior(const gd::String& name) const {
  return *behaviors.find(name)->second;
}

bool Object::HasBehaviorNamed(const gd::String& name) const {
  return behaviors.find(name) != behaviors.end();
}

#if defined(GD_IDE_ONLY)
std::map<gd::String, gd::PropertyDescriptor> Object::GetProperties(
    gd::Project& project) const {
  std::map<gd::String, gd::PropertyDescriptor> nothing;
  return nothing;
}

gd::BehaviorContent* Object::AddNewBehavior(gd::Project& project,
                                     const gd::String& type,
                                     const gd::String& name) {
  std::unique_ptr<gd::Behavior> behavior =
      project.GetCurrentPlatform().GetBehavior(type); //TODO

  if (behavior) {
    auto behaviorContent = gd::make_unique<gd::BehaviorContent>(name, type);
    behavior->InitializeContent(behaviorContent->GetContent());
    behaviors[name] = std::move(behaviorContent);
    return behaviors[name].get();
  } else {
    return nullptr;
  }
}

std::map<gd::String, gd::PropertyDescriptor>
Object::GetInitialInstanceProperties(const gd::InitialInstance& instance,
                                     gd::Project& project,
                                     gd::Layout& layout) {
  std::map<gd::String, gd::PropertyDescriptor> nothing;
  return nothing;
}
#endif

void Object::UnserializeFrom(gd::Project& project,
                             const SerializerElement& element) {
  // Name and type are already loaded.
  objectVariables.UnserializeFrom(
      element.GetChild("variables", 0, "Variables"));
  behaviors.clear();

  // Compatibility with GD <= 3.3
  if (element.HasChild("Automatism")) {
    for (std::size_t i = 0; i < element.GetChildrenCount("Automatism"); ++i) {
      SerializerElement& behaviorElement = element.GetChild("Automatism", i);

      gd::String type =
          behaviorElement.GetStringAttribute("type", "", "Type")
              .FindAndReplace("Automatism", "Behavior");
      gd::String name =
          behaviorElement.GetStringAttribute("name", "", "Name");

      auto behaviorContent = gd::make_unique<gd::BehaviorContent>(name, type);
      behaviorContent->UnserializeFrom(behaviorElement);
      behaviors[name] = std::move(behaviorContent);
    }
  }
  // End of compatibility code
  else {
    SerializerElement& behaviorsElement =
        element.GetChild("behaviors", 0, "automatisms");
    behaviorsElement.ConsiderAsArrayOf("behavior", "automatism");
    for (std::size_t i = 0; i < behaviorsElement.GetChildrenCount(); ++i) {
      SerializerElement& behaviorElement = behaviorsElement.GetChild(i);

      gd::String type =
          behaviorElement.GetStringAttribute("type").FindAndReplace(
              "Automatism", "Behavior");  // Compatibility with GD <= 4
      gd::String name = behaviorElement.GetStringAttribute("name");

      auto behaviorContent = gd::make_unique<gd::BehaviorContent>(name, type);
      behaviorContent->UnserializeFrom(behaviorElement);
      behaviors[name] = std::move(behaviorContent);
    }
  }

  DoUnserializeFrom(project, element);
}

#if defined(GD_IDE_ONLY)
void Object::SerializeTo(SerializerElement& element) const {
  element.SetAttribute("name", GetName());
  element.SetAttribute("type", GetType());
  objectVariables.SerializeTo(element.AddChild("variables"));

  SerializerElement& behaviorsElement = element.AddChild("behaviors");
  behaviorsElement.ConsiderAsArrayOf("behavior");
  std::vector<gd::String> allBehaviors = GetAllBehaviorNames();
  for (std::size_t i = 0; i < allBehaviors.size(); ++i) {
    SerializerElement& behaviorElement = behaviorsElement.AddChild("behavior");

    behaviorElement.SetAttribute("type",
                                 GetBehavior(allBehaviors[i]).GetTypeName());
    behaviorElement.SetAttribute("name",
                                 GetBehavior(allBehaviors[i]).GetName());
    GetBehavior(allBehaviors[i]).SerializeTo(behaviorElement);
  }

  DoSerializeTo(element);
}
#endif

}  // namespace gd
