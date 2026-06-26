#include "Class.h"

APlayerController* UGameplayStatics::GetPlayerControllerFromID(uintptr_t WorldContextObject, int ControllerId)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.GameplayStatics.GetPlayerControllerFromID"));
   }	
   struct {
        uintptr_t WorldContextObject;
        int ControllerId;        
        APlayerController* ReturnValue;
   } Params;
   Params.WorldContextObject = WorldContextObject;   
   Params.ControllerId = ControllerId;
   ProcessEvent(this, (void*)pFunc, &Params);   
   return Params.ReturnValue;
}
void UGameplayStatics::GetAllActorsOfClass(uintptr_t WorldContextObject, AActor* ActorClass, TArray<class AActor*>* OutActors)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GetAllActorsOfClass"));
   }	
   struct {
        uintptr_t WorldContextObject;
        AActor* ActorClass;
        TArray<class AActor*> OutActors;
   } Params;
   Params.WorldContextObject = WorldContextObject;   
   Params.ActorClass = ActorClass;
   ProcessEvent(this, (void*)pFunc, &Params);   
   if (OutActors != nullptr) {
       *OutActors = Params.OutActors;
   }
}
bool KismetSystemLibrary::LineTraceSingle(uintptr_t WorldContextObject, FVector Start, FVector End, int TraceChannel, bool bTraceComplex, TArray<AActor*> ActorsToIgnore, int DrawDebugType, HitResult OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.KismetSystemLibrary.LineTraceSingle"));
   }		
   struct {
        uintptr_t WorldContextObject;
        FVector Start;
        FVector End;
        int TraceChannel;
        bool bTraceComplex;
        TArray<AActor*> ActorsToIgnore;
        int DrawDebugType;
        HitResult OutHit;
        bool bIgnoreSelf;
        FLinearColor TraceColor;
        FLinearColor TraceHitColor;
        float DrawTime;
        bool ReturnValue;
   } Params;
   
   Params.WorldContextObject = WorldContextObject;
   Params.Start = Start;
   Params.End = End;
   Params.TraceChannel = TraceChannel;
   Params.bTraceComplex = bTraceComplex;
   Params.ActorsToIgnore = ActorsToIgnore;
   Params.DrawDebugType = DrawDebugType;
   Params.bIgnoreSelf = bIgnoreSelf;
   Params.TraceColor = TraceColor;
   Params.TraceHitColor = TraceHitColor;
   Params.DrawTime = DrawTime;   
   ProcessEvent(this, (void*)pFunc, &Params);	   
   OutHit = Params.OutHit;
   return Params.ReturnValue;
}
AActor* APlayerController::K2_GetPawn()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.Controller.K2_GetPawn"));
   }	
   struct {
		AActor* ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
bool APlayerController::ProjectWorldLocationToScreen(FVector WorldLocation, FVector2D &ScreenLocation, bool bPlayerViewportRelative)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.PlayerController.ProjectWorldLocationToScreen"));
   }	
   struct {
        FVector WorldLocation;
        FVector2D ScreenLocation;
        bool bPlayerViewportRelative;    
		bool ReturnValue;
   } Params;
   Params.WorldLocation = WorldLocation;
   Params.bPlayerViewportRelative = bPlayerViewportRelative;   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   ScreenLocation = Params.ScreenLocation;
   return Params.ReturnValue;
}
void APlayerController::SetControlRotation(const FRotator& NewRotation)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.Controller.SetControlRotation"));
   }	
   struct {
        FRotator NewRotation;
   } Params;
   Params.NewRotation = NewRotation;   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
}
FVector APlayerCameraManager::GetCameraLocation()
{    
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.PlayerCameraManager.GetCameraLocation"));
   }	
   struct {
		FVector ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;  
}
FString APlayerState::GetPlayerName()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.PlayerState.GetPlayerName"));
   }	
   struct {
		FString ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;  
}
FVector AActor::K2_GetActorLocation()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.Actor.K2_GetActorLocation"));
   }	
   struct {
		FVector ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
FVector AActor::GetVelocity()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.Actor.GetVelocity"));
   } 
   struct {        
        FVector ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
bool AActor::IsDead()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.CharacterBase.IsDead"));
   }	
   struct {
		bool ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
bool AActor::IsAI()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPCharacterBase.IsAI"));
   }	
   struct {
		bool ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
WeaponBase* AActor::GetMainWeapon()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPCharacterBase.GetMainWeapon"));
   }	
   struct {
		WeaponBase* ReturnValue;
   } Params;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
float GPHealthDataComponent::GetHealth()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPHealthDataComponent.GetHealth"));
   }	
   struct {
		float ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
float GPHealthDataComponent::GetHealthMax()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPHealthDataComponent.GetHealthMax"));
   }	
   struct {
		float ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}   
GPTeamComponent* AActor::GetTeamComp()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPCharacterBase.GetTeamComp"));
   }	
   struct {
		GPTeamComponent* ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
GPHealthDataComponent* AActor::GetHealthComp()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPCharacterBase.GetHealthComp"));
   }	
   struct {
		GPHealthDataComponent* ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
int GPTeamComponent::GetTeamIndex()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPTeamComponent.GetTeamID"));
   }	
   struct {
		int ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
int GPTeamComponent::GetCamp()
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("GPGameplay.GPTeamComponent.GetCamp"));
   }	
   struct {
		int ReturnValue;
   } Params;
   
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
FName USkeletalMeshComponent::GetBoneName(int BoneIndex)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("Engine.SkinnedMeshComponent.GetBoneName"));
   }	
   struct {
        int BoneIndex;
		FName ReturnValue;
   } Params;
   Params.BoneIndex = BoneIndex;
   if (this != nullptr) {
      ProcessEvent(this, (void*)pFunc, &Params);
   }
   return Params.ReturnValue;
}
FVector USkeletalMeshComponent::GetBoneLocation(FName BoneName, char Space)
{
    auto function = reinterpret_cast<FVector (*)(void *, FName, char)>(GameBase + Offset::GetBoneLocation);
    return function(this, BoneName, Space);   
}

static uintptr_t GetCharacterEquipComponent(AActor* Owner)
{
   static uintptr_t pFunc = 0;
   if (!pFunc){
       pFunc  = StaticFindObject<uintptr_t>(xorstr_("DFMGameplay.CharacterEquipComponent.Get"));
   }	
   struct {
		uintptr_t ReturnValue;
   } Params;
   static uintptr_t pthis = 0;
   if (!pthis){
       pthis  = StaticFindObject<uintptr_t>(xorstr_("CharacterEquipComponent"));
   }	      
   ProcessEvent((void*)pthis, (void*)pFunc, &Params);
   
   return Params.ReturnValue;
}
