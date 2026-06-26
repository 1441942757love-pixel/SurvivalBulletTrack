class GPHealthDataComponent;
class GPTeamComponent;
class APlayerController;
class WeaponBase;
class AActor;

class HitResult {
    public:       
        char pad_88[0x88];
};
class UGameplayStatics {
    public:
    	static UGameplayStatics* StaticClass()
    	{
            static UGameplayStatics* pStaticClass = 0;
            if (!pStaticClass)
                pStaticClass = StaticFindObject<UGameplayStatics *>(xorstr_("Default__GameplayStatics"));
    		return pStaticClass;
    	}
    	APlayerController* GetPlayerControllerFromID(uintptr_t WorldContextObject, int ControllerId);
    	void GetAllActorsOfClass(uintptr_t WorldContextObject, AActor* ActorClass, TArray<AActor*>* OutActors);
};
class KismetSystemLibrary {
    public:
	    static KismetSystemLibrary* StaticClass()
    	{
            static KismetSystemLibrary* pStaticClass = 0;
            if (!pStaticClass)
                pStaticClass = StaticFindObject<KismetSystemLibrary *>(xorstr_("Default__KismetSystemLibrary"));
    		return pStaticClass;
    	}    
        bool LineTraceSingle(uintptr_t WorldContextObject, FVector Start, FVector End, int TraceChannel, bool bTraceComplex, TArray<AActor*> ActorsToIgnore, int DrawDebugType, HitResult OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);
};
class APlayerController {
    public:
        AActor* K2_GetPawn(); 
        bool ProjectWorldLocationToScreen(FVector WorldLocation, FVector2D &ScreenLocation, bool bPlayerViewportRelative);
        void SetControlRotation(const FRotator& NewRotation);

};
class APlayerCameraManager {
    public:
        FVector GetCameraLocation();        
};
class APlayerState {
    public:            
        FString GetPlayerName();
};
class AActor {
    public:            
    	static AActor* StaticClass()
    	{
            static AActor* pStaticClass = 0;
            if (!pStaticClass)
                pStaticClass = StaticFindObject<AActor* >(xorstr_("Character"));
    		return pStaticClass;
    	}        	
    	
    	static AActor* DeadBox()
    	{
            static AActor* pStaticClass = 0;
            if (!pStaticClass)
                pStaticClass = StaticFindObject<AActor* >(xorstr_("InventoryPickup_DeadBody"));
    		return pStaticClass;
    	}        	    	
    	
    	static AActor* Inventory()
    	{
            static AActor* pStaticClass = 0;
            if (!pStaticClass)
                pStaticClass = StaticFindObject<AActor* >(xorstr_("InventoryPickup"));
    		return pStaticClass;
    	}        	    	    	
    	FVector K2_GetActorLocation(); 
    	FVector GetVelocity();
        bool IsAI();
        bool IsDead();   	
        WeaponBase* GetMainWeapon();
    	GPTeamComponent* GetTeamComp();
    	GPHealthDataComponent* GetHealthComp();
};

class USkeletalMeshComponent {
    public:            
        FName GetBoneName(int BoneIndex);
        FVector GetBoneLocation(FName BoneName, char Space);
};

class GPTeamComponent {
    public:            
        int GetTeamIndex();
        int GetCamp();
};
class GPHealthDataComponent {
    public:            
        float GetHealthMax();
        float GetHealth();
};
class WeaponBase {
    public:       
        int GetWeaponId();
};
