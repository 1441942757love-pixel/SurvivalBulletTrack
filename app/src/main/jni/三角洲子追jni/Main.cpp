#include "include.h"

static FVector CalcTargetLoc(AActor* TargetPlayer, FVector StartLocaion) 
{
    FVector TargetMoveDir = TargetPlayer->GetVelocity();
          
    auto WeaponBase = GetPointer(GetPointer((uintptr_t)AcknowledgedPawn + Offset::CacheCurWeapon) + Offset::WeaponDataComponentFiring);         
      
    float BulletSpeed = GetFloat(WeaponBase + Offset::BulletSpeed);
    if (BulletSpeed == 0.0f) {
        BulletSpeed = 75000.0f;
    }

    double aimoSpeed = BulletSpeed * 0.01f;    

    double distance = sqrtf(powf(TrackLockPos.X - StartLocaion.X, 2.0f) + powf(TrackLockPos.Y - StartLocaion.Y, 2.0f) + powf(TrackLockPos.Z - StartLocaion.Z, 2.0f));
  
    //到达耗时
    double flyTime = distance / aimoSpeed; //秒	
    FVector AnticipationPoint;
    //飞行和修改耗时
    AnticipationPoint.X = TrackLockPos.X + TargetMoveDir.X * flyTime;
    AnticipationPoint.Y = TrackLockPos.Y + TargetMoveDir.Y * flyTime;
    AnticipationPoint.Z = TrackLockPos.Z + TargetMoveDir.Z * flyTime;
    
    double AnticipationDistance = sqrtf(powf(AnticipationPoint.X - StartLocaion.X, 2.0f) + powf(AnticipationPoint.Y - StartLocaion.Y, 2.0f) +powf(AnticipationPoint.Z - StartLocaion.Z, 2.0f)) * 0.01f;
    double realFlyTime = AnticipationDistance / aimoSpeed;    
    AnticipationPoint.X = TrackLockPos.X + TargetMoveDir.X * realFlyTime;
    AnticipationPoint.Y = TrackLockPos.Y + TargetMoveDir.Y * realFlyTime;
    AnticipationPoint.Z = TrackLockPos.Z + TargetMoveDir.Z * realFlyTime;      
    
    return AnticipationPoint;                           
}

static void SetAimLocation(FVector ObjInfo, float distance) {
    
	if (Settings.自动压枪) {
	    ObjInfo.Z -= distance * Settings.压枪力度;   
	}	
        
    double vax = 180.0f / M_PI;
    double vay = 180.0f / M_PI;
    double cx = ObjInfo.X - CameraLoc.X;
    double cy = ObjInfo.Y - CameraLoc.Y;
    double cz = ObjInfo.Z - CameraLoc.Z;
    double pfg = sqrtf((cx * cx) + (cy * cy));    
   
    int bIsFire = Ram<int>((uintptr_t)AcknowledgedPawn + Offset::IsFiring);               
    
    if (TempLockVisble && Settings.开火自瞄 && (bIsFire)) {        
        double XGX, XGY;     
        XGX = atan2f(cy, cx) * vax;
        XGY = atan2f(cz, pfg) * vay;
       
        if (XGX >= -180.0f && XGX < 0.0f) {
            XGX = XGX + 360.0f;
        }
        if (XGY < 0.0f && XGY >= -180.0f) {
            XGY = XGY + 360.0f;
        }
        
        if (XGY >= 360.0f) {
            XGY = XGY - 360.0f;
        }
        if (XGY < 0.0f) {
            XGY = XGY + 360.0f;
        }
        FRotator currentRotation = Ram<FRotator>((uintptr_t)PlayerController + Offset::ControllerRotator);
        float needAddY = XGY - currentRotation.X;//P
        float needAddX = XGX - currentRotation.Y;//Y
        //358 = -2
        //181 = -179
        if (needAddX > 180.0f) {
            needAddX = needAddX - 360.0f;
        }
        //-358 = 2
        //-181 = 179
        if (needAddX < -180.0f) {
            needAddX = 360.0f + needAddX;
        }
        //358 = -2
        //181 = -179
        if (needAddY > 180.0f) {
            needAddY = needAddY - 360.0f;
        }
        //-358 = 2
        //-181 = 179
        if (needAddY < -180.0f) {
            needAddY = 360.0f + needAddY;
        }

        needAddX = needAddX / Settings.拉枪速度;
        needAddY = needAddY / Settings.拉枪速度 * (Settings.拉枪速度 == 1.0f ? 1.0f : 2.0f);

        XGX = currentRotation.Y + needAddX;
        XGY = currentRotation.X + needAddY;
       
        if (XGX >= 360.0f) {
            XGX = XGX - 360.0f;
        }
        if (XGX < 0.0f) {
            XGX = XGX + 360.0f;
        }
        if (XGY >= 360.0f) {
            XGY = XGY - 360.0f;
        }
        if (XGY < 0.0f) {
            XGY = XGY + 360.0f;
        }        
        PlayerController->SetControlRotation({(float)XGY, (float)XGX, 0});  
    }
}

uintptr_t (*orig_sub)(uintptr_t a1, V4D *a2, float *a3, uintptr_t *a4, uintptr_t *a5, uintptr_t a6, uintptr_t a7);
uintptr_t new_sub(uintptr_t a1, V4D *a2, float *a3, uintptr_t *a4, uintptr_t *a5, uintptr_t a6, uintptr_t a7) {   
    if (Settings.范围)
    {
        auto u64 = GetPointer(a1 + 0x20);
        if (GetDword(u64 + 0x18) == 1)
        {
            if (GetByte(u64 + 0x48) == 53 && GetByte(u64 + 0x4C) == 75)
            {
                *a3 = Settings.头范围; 
            }
            if (GetFloat(u64 + 0x48) == 22.75)
            {
                *a3 = Settings.身范围; 
            }
        }
    }
    return orig_sub(a1, a2, a3, a4, a5, a6, a7);
}

void DrawPlayer()
{
    int ActorCount = 0;      
    float TempDis = 9999;    
    AActor* TempObj = 0;   

    if (Settings.范围) {
        if (!initHook) {
            InlineHook((void *)(GameBase + Offset::CollisionBody), (void *)new_sub, (void **)&orig_sub);
            initHook = true;
        }    
    }
    
            
    if (World)
    {
        UGameplayStatics *gGameplayStatics = (UGameplayStatics *)UGameplayStatics::StaticClass();
        if (gGameplayStatics) 
		{
        
            PlayerController = gGameplayStatics->GetPlayerControllerFromID(World, 0);                                   
            if (PlayerController) 
			{
            
                AcknowledgedPawn = PlayerController->K2_GetPawn();   
                //LOGD("自身 %lx 武器: %lx",AcknowledgedPawn, AcknowledgedPawn->GetMainWeapon());
                
                if (GetFloat((uintptr_t)AcknowledgedPawn + Offset::CrouchedEyeHeight) == 40.0f)
                {            
                    PlayerCameraManager = (APlayerCameraManager *)GetPointer((uintptr_t)PlayerController + Offset::PlayerCameraManager);
                    CameraLoc = PlayerCameraManager->GetCameraLocation();  
    
                    TArray<AActor *> PlayerCharacters;
                    gGameplayStatics->GetAllActorsOfClass(World, AActor::StaticClass(), &PlayerCharacters);       
                    ActorsToIgnore = PlayerCharacters;          
                    
                    for (int i = 0; i < PlayerCharacters.Num(); i++)
                    {                                                            
                        auto Player = PlayerCharacters[i];    
            
                        if (!Player || Player == AcknowledgedPawn)
                            continue;                                           
                        
                        if (GetFloat((uintptr_t)Player + Offset::CrouchedEyeHeight) != 40.0f)
                            continue;          
                            
                        bool bIsAI = Player->IsAI();            
                                      
                        auto PlayerLocation = Player->K2_GetActorLocation();               
                        if (PlayerLocation.X == 0 || PlayerLocation.Y == 0 || PlayerLocation.Z == 0)
                            continue;
                                                                      
                        float Distance = GetDistance(CameraLoc, PlayerLocation);  
                        if (Distance > 400) 
                            continue;           
                                   
                        auto Mesh = (USkeletalMeshComponent *)GetPointer((uintptr_t)Player + Offset::Mesh);
                        if (!Mesh) 
                            continue;      
                                                
                        bool bDead = Player->IsDead();    
                        if (bDead)  
                            continue; 
                                
                        float Health = Player->GetHealthComp()->GetHealth() / Player->GetHealthComp()->GetHealthMax() * 100;     

                        int SelfTeam, SelfCamp, TeamId, CampId;                        
                        auto SelfTeamComp = AcknowledgedPawn->GetTeamComp();
                        if (SelfTeamComp) {
                            SelfTeam = SelfTeamComp->GetTeamIndex();
                            SelfCamp = SelfTeamComp->GetCamp();
                        }                                    
                        auto TeamComp = Player->GetTeamComp();
                        if (TeamComp) {
                            TeamId = TeamComp->GetTeamIndex();
                            CampId = TeamComp->GetCamp();
                        }           
                        if (Settings.战场) {                
                            if (SelfTeam == TeamId || SelfCamp == CampId || Health == 0.0)
                                continue;                  
                        } else {
                            if (SelfTeam == TeamId)
                                continue;                                            
                        }                
                        
                        if (bIsAI && Settings.隐藏人机)
                            continue;         
                            
                        ActorCount ++;
                                    
                        FVector outHead;
                        FVector2D outPelvis, outLeft_Ankle, outRight_Ankle;                            
                        auto HeadPos = GetBoneLocation(Mesh, 31);                   
                        auto PelvisPos = GetBoneLocation(Mesh, 1);                   
                        HeadPos.Z += 7;                      

                        TempLockVisble = false;   
                        
                        if (BoneToScreen_LineOfSight(HeadPos, &outHead) && BoneToScreen(PelvisPos, &outPelvis)) 
                        {
                                                                                                     
                            FVector2D outLeft_Ankle, outRight_Ankle;                                    
                            auto Left_AnklePos = GetBoneLocation(Mesh, 60);        
                            auto Right_AnklePos = GetBoneLocation(Mesh, 64);   
                            BoneToScreen(Left_AnklePos, &outLeft_Ankle); BoneToScreen(Right_AnklePos, &outRight_Ankle);
                            
                            FVector outScreen;
                            WorldToScreen(PlayerLocation, &outScreen);          
                                                        
                            float Top = outPelvis.Y - (outPelvis.Y - outHead.Y) - (outScreen.Z / 3);              
                            float Bottom = CalcBottom(outLeft_Ankle.Y, outRight_Ankle.Y, outScreen.Z);        
                                                                          
                            FVector rect_0 = FVector();
                            FVector rect_1 = FVector();
                            bool ChestLocked = false;            
                            
                            for (int i = 0; i < bonecount; ++i)
                            {                               
                                auto Start = GetBoneLocation(Mesh, bonelist[i][0]);
                                auto End = GetBoneLocation(Mesh, bonelist[i][1]);                   
            
                                if (BoneToScreen_LineOfSight(Start, &rect_0) && BoneToScreen_LineOfSight(End, &rect_1))
                                {                     
                                    if (Settings.骨骼) {
                                        ImGui::GetBackgroundDrawList()->AddLine({rect_0.X, rect_0.Y}, {rect_1.X, rect_1.Y}, rect_0.Z ? (bIsAI ? ImColor(255, 255, 255) : ImColor(255, 0, 0)) : (bIsAI ? ImColor(0, 255, 0) : ImColor(0, 255, 0)), 1.5f);
                                    }
                                }              
                                if (Settings.Index01 > 0)
                                {
                                    if (!ChestLocked && Settings.Index01 == 1 && bonelist[i][0] == Chest && rect_0.Z == 1.f) // 胸部优先
                                    {
                                        TempLockPos = Start; TempLockVisble = true; ChestLocked = true;
                                    }
                                    else if (Settings.Index01 == 2 && rect_0.Z == 1.f) // 指哪打哪
                                    {
                                        // 判断是否更近
                                        float dx = fabsf(screenWidth / 2 - rect_0.X);
                                        float dy = fabsf(screenHeight / 2 - rect_0.Y);
                                        float dis = dx + dy;
                                        if (dis < TempDis)
                                        {
                                            TempDis = dis;
                                            TempLockPos = Start; TempLockVisble = true;
                                        }
                                    }
                                }                                                                                                                  
                            }        
                            
                            if ((Settings.骨骼) && outHead.X != 0.0 && outHead.Y != 0.0) {                        
                                ImGui::GetBackgroundDrawList()->AddCircle({outHead.X, outHead.Y}, outScreen.Z / 12, outHead.Z ? (bIsAI ? ImColor(255, 255, 255) : ImColor(255, 0, 0)) : (bIsAI ? ImColor(0, 255, 0) : ImColor(0, 255, 0)), 0.0f, 1.5f);
                            }                                            
                            if (Settings.方框) {                       
                                ImGui::GetBackgroundDrawList()->AddRect({outHead.X - (outScreen.Z / 2), Top}, {outHead.X + (outScreen.Z / 2), Bottom}, TempLockVisble ? (bIsAI ? ImColor(255, 255, 255) : ImColor(255, 0, 0)) : (bIsAI ? ImColor(0, 255, 0) : ImColor(0, 255, 0)), 2.0f, 0, 1.5f);                                       
                            }       
                            if (Settings.射线) {
                                ImGui::GetBackgroundDrawList()->AddLine({(float)screenWidth / 2, 0}, {outHead.X, Top - 20}, TempLockVisble ? (bIsAI ? ImColor(255, 255, 255) : ImColor(255, 0, 0)) : (bIsAI ? ImColor(0, 255, 0) : ImColor(0, 255, 0)), 1.5f);                              
                            }                 
                            if (!bIsAI) {
                				if (Settings.信息) {
                				    char PlayerName[256];
                                    ReadName(PlayerName,  GetPointer(GetPointer((uintptr_t)Player + Offset::PlayerState) + Offset::PlayerName));      				                 	
                					string s;
                					s += "[";
                					s += to_string((int)TeamId);
                					s += "]";                      	
                					s += PlayerName;          
		                            s += "(";
                					s += to_string((int)Distance);
                					s += "M)";                          					  					
                                    DrawText(25, outHead.X, Bottom + 5, TempLockVisble ? 暗红色 : 暗绿色, s.c_str());                		            	                				
                                }                 
                                if (Settings.英雄) {
                                    int HeroID = GetDword(GetPointer((uintptr_t)Player + Offset::PlayerState) + Offset::HeroID); 
	                                string s;
                					s += "[";
                					s += GetHeroName(HeroID);
                					s += "]";                      	                                
                                    DrawText(25, outHead.X, Bottom + 25, TempLockVisble ? 暗红色 : 暗绿色, s.c_str());                		            	               
                                }
                                if (Settings.护甲) {               			                                    
                                    int Helmet = GetDword(GetPointer(GetPointer((uintptr_t)Player + Offset::CharacterEquipComponentCache) + Offset::EquipedArmorInfoArray) + Offset::Helmet);
                                    int Armor = GetDword(GetPointer(GetPointer((uintptr_t)Player + Offset::CharacterEquipComponentCache) + Offset::EquipedArmorInfoArray) + Offset::Armor);    					
            						string Text;
            						Text += std::to_string((int)GetArmorLeveL(Helmet));
            						Text += "头";
            						Text += "/";
            						Text +=  std::to_string((int)GetArmorLeveL(Armor));
            						Text += "甲";                   			   			
                                    DrawText(25, outHead.X, Bottom + 45, TempLockVisble ? 暗红色 : 暗绿色, Text.c_str());                 	            	                							            	                						
                				}
                				if (Settings.手持) {
                                    auto WeaponId = GetDword(GetPointer((uintptr_t)Player + Offset::CacheCurWeapon) + Offset::WeaponID);
            						string Text;
            						Text += GetWeapon(WeaponId);    	            				
                                    DrawText(25, outHead.X, Bottom + 65, TempLockVisble ? 暗红色 : 暗绿色, Text.c_str());                                                 	                				                				                				                				
                				}                                        		                                
                            } else {                            
		                        if (Settings.信息) {               			        	
                					string s;
		                            s += "AI(";
                					s += to_string((int)Distance);
                					s += "M)";                          					  					
                                    DrawText(25, outHead.X, Bottom + 5, TempLockVisble ? 暗红色 : 暗绿色, s.c_str());                		            	                				
                                }                                             
                            }                              
                                                  
                            if (Settings.血量) {
                                ImColor 血量颜色;
                                float ReaIHealth = Health;
                                if (ReaIHealth >= 80) {
                                    血量颜色 = ImColor(10, 240, 10, 210);
                                } else if (ReaIHealth >= 60) {
                                    血量颜色 = ImColor(255, 255, 0, 210);    
                                } else if (ReaIHealth >= 30) {
                                    血量颜色 = ImColor(255, 0, 0, 210);      
                                } else {
                                    血量颜色 = ImColor(128, 0, 0, 210);       
                                }
                                
                                ImGui::GetForegroundDrawList()->AddRectFilled({outHead.X - 20, Top - 15}, {outHead.X - 20 + 0.4f * ReaIHealth, Top - 5},血量颜色);
                                ImGui::GetForegroundDrawList()->AddRect({outHead.X - 20,Top - 15}, {outHead.X + 20,Top - 5}, ImColor(0, 0, 0), 0, 0, 0.8);                                                 
                            }                                                                   
                        }            
                        if (Settings.子弹追踪 || Settings.开火自瞄)
                        {                       
                           FVector2D outpos = {0,0};
                           BoneToScreen(TempLockPos, &outpos);
                           float DisX = abs(screenWidth / 2 - outpos.X);
                           float DisY = abs(screenHeight / 2 - outpos.Y);                
                           if (DisX <= Settings.锁定范围 && DisY <= Settings.锁定范围 && TempLockVisble == true)
                           {   
                               auto Dis = (DisX + DisY) / 2;
                               if (Dis < TempDis)
                               {
                                    if (Settings.忽略倒地 && Health <= 0)
                                    {
                                    }         
                                    else if (Settings.忽略人机 && bIsAI)
                                    {
                                    }                                                          
                                    else
                                    {
                                       TempDis = Dis;
                                       TempObj = Player;
                                    }
                               }                                                     
                               if ((Settings.子弹追踪 || Settings.开火自瞄) && LockObj == Player) {                                                                       
                                    TrackLockPos = TempLockPos;          
                                    if (Settings.启用预判) {
                                        TargetLoc = CalcTargetLoc(Player, CameraLoc);
                                    } else {
                                        TargetLoc = TempLockPos;
                                    }       
                                    SetAimLocation(TargetLoc, Distance);                                     
                                    ImGui::GetBackgroundDrawList()->AddLine({(float)screenWidth / 2, (float)screenHeight / 2}, {outpos.X, outpos.Y}, ImColor(0, 255, 255), 1.5f);                                                                                                                                                                           
                        	   }    			           
                           }
                        }                 
                        if (Settings.子弹瞬击) {
                            if (AcknowledgedPawn) {
                                auto WeaponBase = Ram<uint64_t>((uintptr_t)AcknowledgedPawn + Offset::CacheCurWeapon);
                                if (WeaponBase) {
                                    auto WeaponBulletSpeed = Ram<uint64_t>(WeaponBase + Offset::WeaponDataComponentFiring);
                                    if (WeaponBulletSpeed) {
                                        auto A1 = WeaponBulletSpeed + Offset::InitBulletSpeed;
                                        auto A2 = WeaponBulletSpeed + Offset::InitBulletSpeed + 0x4;
                                        auto A3 = WeaponBulletSpeed + Offset::InitBulletSpeed + 0x8;
                                        auto A4 = WeaponBulletSpeed + Offset::InitBulletSpeed + 0xC;
                                        auto A6 = WeaponBulletSpeed + Offset::InitBulletSpeed + 0x10;
                                        auto A5 = WeaponBulletSpeed + Offset::InitBulletSpeed + 0x14;
                                        if (Ram<float>(A1 - 0x4) == 1.f && Ram<float>(A1) != 100000000) {
                                            WriteFloat(A1, 100000000);
                                            WriteFloat(A2, 100000000);
                                            WriteFloat(A3, 100000000);
                                            WriteFloat(A4, 100000000);
                                            WriteFloat(A5, 100000000);
                                            WriteFloat(A6, 100000000);                        
                                        }    
                                    }
                                }
                            }                            
                        }                                                                       
                    }                          
                     
                    if (Settings.盒子) {                   
                        TArray<AActor *> PlayerDeadBox;
                        gGameplayStatics->GetAllActorsOfClass(World, AActor::DeadBox(), &PlayerDeadBox);      
                                          
                        for (int i = 0; i < PlayerDeadBox.Num(); i++)
                        {                                                            
                            auto Object = PlayerDeadBox[i];    
                
                            if (!Object)
                                continue;                                           
                            
                            auto Location = Object->K2_GetActorLocation();               
                            if (Location.X == 0 || Location.Y == 0 || Location.Z == 0)
                                continue;

                            float Distance = GetDistance(CameraLoc, Location);  
                            if (Distance > 200) 
                                continue;           
                                
                            FVector2D objScreen;
                            if (!BoneToScreen(Location, &objScreen))
                                continue;                
                                
                            std::string s;
                            s += "盒子";
                            s += "[";   
                            s += std::to_string((int)Distance);            
                            s += "米]";                      
                            DrawText(25, objScreen.X, objScreen.Y, Settings.盒子颜色, s.c_str());                                                                                                                                                             
                        }                                                           
                    }               
                    
                    if (Settings.物资) {                   
                        TArray<AActor *> InventoryArray;
                        gGameplayStatics->GetAllActorsOfClass(World, AActor::Inventory(), &InventoryArray);      
                                          
                        for (int i = 0; i < InventoryArray.Num(); i++)
                        {                                                            
                            auto Object = InventoryArray[i];    
                
                            if (!Object)
                                continue;                                           
                            
                            auto Location = Object->K2_GetActorLocation();               
                            if (Location.X == 0 || Location.Y == 0 || Location.Z == 0)
                                continue;

                            float Distance = GetDistance(CameraLoc, Location);  
                            if (Distance > 200) 
                                continue;           
                                
                            FVector2D objScreen;
                            if (!BoneToScreen(Location, &objScreen))
                                continue;                
                            
                            
                            int ItemValue = GetDword(GetPointer((uintptr_t)Object + Offset::ValueComponent) + Offset::ItemValue);              
                            if (ItemValue <= 0)
                                continue; 
                            
                            if (ItemValue > Settings.最低过滤价值) {
                                ImColor 物资颜色;
                                
                                if (ItemValue <= 10000) {
                                    物资颜色 = ImColor(0, 255, 0); // 绿色
                                } else if (ItemValue <= 150000) {
                                    物资颜色 = ImColor(128, 0, 128); // 紫色
                                } else if (ItemValue <= 450000) {
                                    物资颜色 = ImColor(255, 255, 0); // 黄色
                                } else {
                                    物资颜色 = ImColor(255, 0, 0); // 红色
                                }
                                
                                std::string s;
                                s += "物品";
                                s += "[";   
                                s += std::to_string((int)ItemValue);            
                                s += "＄]";             
                                s += " ";   
                                s += std::to_string((int)Distance);            
                                s += "米";        							                            
                                DrawText(25, objScreen.X, objScreen.Y, 物资颜色, s.c_str());           
                            }
                            
                            // static char info1[128];
                            // sprintf(info1,"%lx", Object);                                                                                                                                                        
                        }                                                           
                    }                                                                                                                                                                                                                                                                                                    
                }
            }
        }
    }

	if (Settings.子弹追踪 || Settings.开火自瞄) {
		ImGui::GetBackgroundDrawList()->AddCircle({(float)screenWidth / 2, (float)screenHeight / 2}, Settings.锁定范围, ImColor(255, 255, 255, 255), 0, 1.5f);
	}	        
	
    if (Settings.人数) {
	    string Text = "";
		if (ActorCount == 0) {
        	Text += "安全";
			ImGui::GetBackgroundDrawList()->AddImage(NoSafe_Picture.textureId, {screenWidth / 2 - 90, 95}, {screenWidth / 2 + 90, 140});
		} else if (ActorCount != 0) {
			Text += to_string((int)ActorCount);
			ImGui::GetBackgroundDrawList()->AddImage(Safe_Picture.textureId, {screenWidth / 2 - 90, 95}, {screenWidth / 2 + 90, 140});
		}
		ImVec2 TextSize = ImGui::CountTextSize(NULL, Text.c_str(), 35);
		ImGui::GetBackgroundDrawList()->AddText(NULL, 35, {screenWidth / 2 - (TextSize.x / 2), 116 - (TextSize.y / 2)}, ImColor(255, 255, 255, 255), Text.c_str());        
    }    
    LockObj = TempObj;
}
void DrawMenu()
{
	if (g_EglDisplay == EGL_NO_DISPLAY)
		return;
		
	eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_WIDTH, &glWidth);
	eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_HEIGHT, &glHeight);
	ImGuiIO & io = ImGui::GetIO();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
	ImGui::NewFrame();
	
	ImGui::SetNextWindowSize(ImVec2(800, 450), ImGuiCond_Once);		
    if (ImGui::Begin(xorstr_("KX追踪")))
    {
        if (ImGui::BeginTabBar(xorstr_("Tab"), ImGuiTabBarFlags_FittingPolicyScroll))
        {
            if (ImGui::BeginTabItem(xorstr_("透视设置"))) {    
                ImGui::Checkbox(xorstr_("绘制方框"), &Settings.方框);   
                ImGui::SameLine();               
                ImGui::Checkbox(xorstr_("绘制射线"), &Settings.射线);
                ImGui::SameLine();
                ImGui::Checkbox(xorstr_("绘制骨骼"), &Settings.骨骼); 
                ImGui::SameLine();               
                ImGui::Checkbox(xorstr_("绘制血量"), &Settings.血量);      

                ImGui::Checkbox(xorstr_("绘制信息"), &Settings.信息);   
                ImGui::SameLine();    
                ImGui::Checkbox(xorstr_("绘制护甲"), &Settings.护甲);             
                ImGui::SameLine();               
                ImGui::Checkbox(xorstr_("绘制手持"), &Settings.手持);   
                ImGui::SameLine();               
                ImGui::Checkbox(xorstr_("绘制人数"), &Settings.人数);   
                
                ImGui::Checkbox(xorstr_("绘制英雄"), &Settings.英雄);                            
                ImGui::SameLine();                                                                                                                                                                                                                        
                ImGui::Checkbox(xorstr_("战场模式"), &Settings.战场);    
                ImGui::SameLine();                                                                                                                                                                                                                                                 
                ImGui::Checkbox(xorstr_("隐藏人机"), &Settings.隐藏人机);                                                       
                                                                                                                                                                   
                ImGui::EndTabItem();	 
            }                      
            if (ImGui::BeginTabItem(xorstr_("物资设置"))) {    
                ImGui::Checkbox(xorstr_("绘制盒子"), &Settings.盒子);                
                ImGui::ColorEdit4(xorstr_("盒子颜色"), (float *) &Settings.盒子颜色); 
                ImGui::Checkbox(xorstr_("绘制物资"), &Settings.物资);            
                ImGui::SliderInt(xorstr_("最低过滤价值"), &Settings.最低过滤价值, 0,100000);                               
                ImGui::EndTabItem();	 
            }                     
            if (ImGui::BeginTabItem(xorstr_("自瞄设置"))) {    
	            ImGui::Checkbox(xorstr_("开火自瞄"), &Settings.开火自瞄);
	            ImGui::SameLine();   
           	    ImGui::Checkbox(xorstr_("自动压枪"), &Settings.自动压枪);
                ImGui::SliderFloat(xorstr_("自瞄压枪:"), &Settings.压枪力度, 0.0f, 0.9f, "%.1f");  	
                ImGui::SliderInt(xorstr_("自瞄拉枪:"), &Settings.拉枪速度, 1, 10);     	
                ImGui::Text(xorstr_("其他参数均同步追踪设置"));			     		
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), xorstr_("自瞄开指哪打哪会锁脚"));						               	    
                ImGui::EndTabItem();	 
            }                                           
            if (ImGui::BeginTabItem(xorstr_("追踪设置"))) {    
                ImGui::Checkbox(xorstr_("子弹追踪"), &Settings.子弹追踪);
                ImGui::SameLine();
                ImGui::Checkbox(xorstr_("启用预判"), &Settings.启用预判);
                ImGui::SameLine();
                ImGui::Checkbox(xorstr_("子弹瞬击"), &Settings.子弹瞬击);
                ImGui::Text(xorstr_("开启瞬击后关闭预判算法"));			     
                if (ImGui::CollapsingHeader(xorstr_("范围/概率"))) {  
                    ImGui::SliderFloat(xorstr_("追踪范围"), &Settings.锁定范围, 80.f, 1024.f, xorstr_("%.2f"));
                    ImGui::SliderFloat(xorstr_("追踪概率"), &Settings.追踪概率, 10.f, 100.f, xorstr_("%.2f"));        
                }
				const char *aimingState[] ={"头部优先", "胸部优先", "指哪打哪"};
				ImGui::Combo(xorstr_("追踪部位"), &Settings.Index01, aimingState, IM_ARRAYSIZE(aimingState));                 
                ImGui::Checkbox(xorstr_("忽略倒地"), &Settings.忽略倒地);           
                ImGui::SameLine();
                ImGui::Checkbox(xorstr_("忽略人机"), &Settings.忽略人机);      
                ImGui::EndTabItem();	 
            }                
            if (ImGui::BeginTabItem(xorstr_("范围设置"))) {    
                ImGui::Checkbox(xorstr_("人物范围"), &Settings.范围);    
                ImGui::SliderFloat(xorstr_("头部范围"), &Settings.头范围, 1.0f, 8.f, xorstr_("%.2f"));
                ImGui::SliderFloat(xorstr_("身体范围"), &Settings.身范围, 1.0f, 8.f, xorstr_("%.2f"));  
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), xorstr_("关闭以后恢复效果无法恢复被改动的内存"));						               	    
                ImGui::EndTabItem();	 
            }                                                             
        }                 
        ImGui::Text(xorstr_("耗时 %.3fms/真实帧率 (%.1fFPS)"), 1000.0f / io.Framerate, io.Framerate);			        
    }
    
    DrawText(40.0f, screenWidth / 2, 50, ImColor(255, 255, 0, 255), "Telegram: @Sakurayyda");
    
    DrawPlayer();
	ImGui::End();	
	ImGui::Render();	
	glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

void Clearup()
{
    if (!isInitImGui)
        return;

    // 销毁 EGL
    if (g_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_EglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(g_EglDisplay, g_EglSurface);
            g_EglSurface = EGL_NO_SURFACE;
        }

        if (g_EglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(g_EglDisplay, g_EglContext);
            g_EglContext = EGL_NO_CONTEXT;
        }

        eglTerminate(g_EglDisplay);
        g_EglDisplay = EGL_NO_DISPLAY;
    }

    isInitImGui = false;
}

int32_t(*orig_onInputEvent) (struct android_app * app, AInputEvent * inputEvent);
int32_t onInputEvent(struct android_app *app, AInputEvent * inputEvent)
{
    ImGui_ImplAndroid_HandleInputEvent(inputEvent,{(float)screenWidth / (float)glWidth, (float)screenHeight / (float)glHeight});
    return orig_onInputEvent(app, inputEvent);
}

void (*orig_trace)(void* a1, int a2, int *a3, int a4, void *a5, int *a6, FVector StartLoc, FRotator ShootRot, FRotator BulletRot);
void trace(void* a1, int a2, int *a3, int a4, void *a5, int *a6, FVector StartLoc, FRotator ShootRot, FRotator BulletRot) {
    if (Settings.子弹追踪 && (LockObj) && (dist(engine) < Settings.追踪概率) && (AcknowledgedPawn != NULL))
    {                   
        FRotator TargetRot = ToRotator(StartLoc, TargetLoc);   
        ShootRot = TargetRot;  
        BulletRot = TargetRot;
    }   
    return orig_trace(a1, a2, a3, a4, a5, a6, StartLoc, ShootRot, BulletRot);
}

void InitDraw(ANativeWindow *Window)
{
    if (isInitImGui)  // 避免重复初始化
        return;

    if (g_App->onInputEvent != onInputEvent) {
        orig_onInputEvent = decltype(orig_onInputEvent)(g_App->onInputEvent);
        g_App->onInputEvent = onInputEvent;
    }

    ANativeWindow_acquire(Window);
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        return;
    }
    eglInitialize(g_EglDisplay, nullptr, nullptr);
    const EGLint egl_attributes[] = {
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    EGLint num_configs = 0;
    eglChooseConfig(g_EglDisplay, egl_attributes, nullptr, 0, &num_configs);
    EGLConfig egl_config;
    eglChooseConfig(g_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
    EGLint egl_format;
    eglGetConfigAttrib(g_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(
        Window, screenWidth, screenHeight, egl_format
    );
    const EGLint egl_context_attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_EglContext = eglCreateContext(g_EglDisplay, egl_config, EGL_NO_CONTEXT, egl_context_attributes);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, egl_config, Window, nullptr);
    eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    if (io.Fonts->Fonts.empty()) {
        ImGui::GetStyle().ScaleAllSizes(1.0f);    
        ImFontConfig font_cfg;
        font_cfg.OversampleH = 1;   // 降低采样，减小显存占用
        font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH  = true;
        io.Fonts->AddFontFromMemoryTTF((void*)Window_data, Window_size, 45.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());    
        io.Fonts->AddFontDefault();
    }
    InitTexture();
    ImGui_ImplAndroid_Init(g_App->window);
    ImGui::StyleColorsLight();
    ImGui_ImplOpenGL3_Init(xorstr_("#version 300 es"));
    ANativeWindow_release(g_NativeWindow);    
    isInitImGui = true;
}

void (*orig_DrawTransition)(uintptr_t GameViewport, uintptr_t Canvas);
void DrawTransition(uintptr_t GameViewport, uintptr_t Canvas) {  
    
    SizeX = GetDword(Canvas + Offset::SizeX);
    SizeY = GetDword(Canvas + Offset::SizeY);   
    World = GetPointer(GameViewport + Offset::World);
    
    if (!g_NativeWindow) {
        g_NativeWindow = GetNativeWindow(DrawView);      
        GameWindow = g_App->window;             
        orig_DrawTransition(GameViewport, Canvas);
        return;
    }
    
    if (g_App->window == GameWindow) {
        InitDraw(g_NativeWindow);
        DrawMenu();
        orig_DrawTransition(GameViewport, Canvas);
        return;
    }
    
    g_NativeWindow = NULL;
    Clearup();
    orig_DrawTransition(GameViewport, Canvas);
}

extern "C"
{
	void android_main(struct android_app* state) {
    	app_dummy();    
	}
}


int handle_message(int fd, __attribute__ ((unused))int events, __attribute__ ((unused))void *data)
{
	Message msg{};
    read(fd, &msg, sizeof(Message));
    if (msg.what == 1 && screenWidth <= 0) {
        DisplayMetrics ScreenInfo = GetScreenInfo();
        screenWidth = ScreenInfo.widthPixels;
        screenHeight = ScreenInfo.heightPixels;
        DrawView = CreateDrawView(ScreenInfo);    
        VirtHook((void**)(GameBase + Offset::PostRender), (void*)DrawTransition, (void**)&orig_DrawTransition, 0);               
        VirtHook((void**)(GameBase + Offset::AimBot), (void*)trace, (void**)&orig_trace, 0);
    }
	return 1;
}

__attribute__((constructor)) int _init() {

	GameBase = GetModule(xorstr_("libUE4.so"));
	while (!GameBase) {
		GameBase = GetModule(xorstr_("libUE4.so"));
		sleep(1);
	}

	    
    g_App = *(android_app **) (GameBase + Offset::GNativeAndroidApp);
	while (!g_App) {
		g_App = *(android_app **) (GameBase + Offset::GNativeAndroidApp);
		sleep(1);
	}   
    
	CurrentJavaVM = g_App->activity->vm;
	
	MainLooper::GetInstance()->init(handle_message);
	
	MainLooper::GetInstance()->send({1, nullptr});
	
	return 0;
}
