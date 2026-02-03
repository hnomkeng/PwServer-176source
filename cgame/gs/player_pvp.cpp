#include "world.h"
#include "player_pvp.h"
#include "clstab.h"

DEFINE_SUBSTANCE(gplayer_pvp_imp,gplayer_imp,CLS_PVP_PLAYER_IMP)
int 
gplayer_pvp_imp::MessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_ATTACK:
			{
				ASSERT(msg.content_length >= sizeof(attack_msg));
				attack_msg & amsg = *(attack_msg*)msg.content;
				if(!(amsg.attacker_mode & attack_msg::PVP_FREE) 
						|| (amsg.attacker_mode & attack_msg::PVP_DUEL))
				{
					return gplayer_imp::MessageHandler(pPlane,msg);
				}
				
				attack_msg ack_msg = *(attack_msg*)msg.content;
				_filters.EF_TransRecvAttack(msg.source, ack_msg);
				ack_msg.is_invader =  false;
				
				XID attacker = ack_msg.ainfo.attacker;
				//�������
				if(attacker.IsPlayerClass())
				{
					if(!ack_msg.force_attack) return 0;

					if(attacker.id == _parent->ID.id ||
							(IsInTeam() && IsMember(attacker))) 
					{
						return 0;
					}

					if(_invader_state == INVADER_LVL_0 && (ack_msg.force_attack & C2S::FORCE_ATTACK_NO_WHITE))
					{
						return 0;
					}

					if(ack_msg.ainfo.mafia_id)
					{
						if(ack_msg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA 
								&& ack_msg.ainfo.mafia_id == ((gplayer*)_parent)->id_mafia) return 0;
						if(ack_msg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA_ALLIANCE 
								&& OI_IsFactionAlliance(ack_msg.ainfo.mafia_id)) return 0;
					}
					
					if(ack_msg.ainfo.force_id && ack_msg.ainfo.force_id == _player_force.GetForce()) return false;

					if(ack_msg.target_faction == 0)
					{
						return 0;
					}
					
					ack_msg.target_faction = 0xFFFFFFFF;
				}

				((gplayer_controller*)_commander)->TrySelect(msg.source);
				HandleAttackMsg(pPlane,msg,&ack_msg);
				return 0;
			}
			break;
		case GM_MSG_ENCHANT:
			{
				ASSERT(msg.content_length >= sizeof(enchant_msg));
				enchant_msg & emsg = *(enchant_msg*)msg.content;
				if(!(emsg.attacker_mode & attack_msg::PVP_FREE)
						|| (emsg.attacker_mode & attack_msg::PVP_DUEL))
				{
					return gplayer_imp::MessageHandler(pPlane,msg);
				}

				enchant_msg ech_msg = *(enchant_msg*)msg.content;
				_filters.EF_TransRecvEnchant(msg.source, ech_msg);
				if(!ech_msg.helpful)
				{
					XID attacker = ech_msg.ainfo.attacker;
					//���Ǹ��ֱ���
					if(attacker.IsPlayerClass())
					{
						if(!ech_msg.force_attack) return 0;

						if(attacker.id == _parent->ID.id ||
								(IsInTeam() && IsMember(attacker))) 
						{
							return 0;
						}

						if(_invader_state == INVADER_LVL_0 && (ech_msg.force_attack & C2S::FORCE_ATTACK_NO_WHITE))
						{
							return 0;
						}

						if(ech_msg.ainfo.mafia_id)
						{
							if(ech_msg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA 
									&& ech_msg.ainfo.mafia_id == ((gplayer*)_parent)->id_mafia) return 0;
							if(ech_msg.force_attack & C2S::FORCE_ATTACK_NO_MAFIA_ALLIANCE 
									&& OI_IsFactionAlliance(ech_msg.ainfo.mafia_id)) return 0;
						}

						if(ech_msg.ainfo.force_id && ech_msg.ainfo.force_id == _player_force.GetForce()) return false;
						
						if(ech_msg.target_faction == 0)
						{
							return 0;
						}
					
						ech_msg.target_faction = 0xFFFFFFFF;
					}

					((gplayer_controller*)_commander)->TrySelect(msg.source);
				}
				else
				{
					XID attacker = ech_msg.ainfo.attacker;
					if(attacker.id != _parent->ID.id)
					{
						if(ech_msg.helpful == 1)
						{
							if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) 
									&& (!IsInTeam() || !IsMember(attacker))) return 0;
						}
						else if(ech_msg.helpful == 2)
						{
							if(_refuse_bless & C2S::REFUSE_NEUTRAL_BLESS) return 0;
						}
					}
				}
				ech_msg.is_invader = false;
				HandleEnchantMsg(pPlane,msg,&ech_msg);
				return 0;
			}
	}
	return gplayer_imp::MessageHandler(pPlane,msg);
}

int 
gplayer_pvp_imp::ZombieMessageHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_SCROLL_RESURRECT:
		{
			EnterResurrectReadyState(0.0f,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
		};
		return 0;

		case GM_MSG_ENCHANT_ZOMBIE:
		{
			ASSERT(msg.content_length >= sizeof(enchant_msg));
			enchant_msg ech_msg = *(enchant_msg*)msg.content;
			if(!ech_msg.helpful)
			{
				return 0;
			}
			else
			{
				XID attacker = ech_msg.ainfo.attacker;
				if(attacker != _parent->ID)
				{
					//��ҿ���ѡ���Ƿ��������ף��
					if(ech_msg.helpful == 1)
					{
						if((_refuse_bless & C2S::REFUSE_NON_TEAMMATE_BLESS) 
								&& (!IsInTeam() || !IsMember(attacker))) return 0;
					}
					else if(ech_msg.helpful == 2)
					{
						if(_refuse_bless & C2S::REFUSE_NEUTRAL_BLESS) return 0;
					}
				}
			}
			ech_msg.is_invader = false;
			HandleEnchantMsg(pPlane,msg,&ech_msg);
		}
		return 0;

		default:
		return gplayer_imp::ZombieMessageHandler(pPlane, msg);
	}
	return 0;
}

void
gplayer_pvp_imp::FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow)
{
	gplayer_imp::FillAttackMsg(target,attack,dec_arrow);
	attack.force_attack |= C2S::FORCE_ATTACK;
	attack.attacker_mode = attack_msg::PVP_FREE;
}

void 
gplayer_pvp_imp::OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead)
{
	gplayer_imp::OnDeath(lastattack, is_pariah, attacker_mode, taskdead);
}

void
gplayer_pvp_imp::OnHeartbeat(unsigned int tick)
{
	if(_parent->IsZombie())
	{
		if(!world_manager::GetWorldLimit().noauto_resurrect)
			SendTo<0>(GM_MSG_GM_RESURRECT,_parent->ID,0);
	}

	gplayer_imp::OnHeartbeat(tick);
}

void
gplayer_pvp_imp::FillEnchantMsg(const XID & target,enchant_msg & enchant)
{
	gplayer_imp::FillEnchantMsg(target,enchant);
	enchant.force_attack |= C2S::FORCE_ATTACK;
	enchant.attacker_mode = attack_msg::PVP_FREE;
}

void 
gplayer_pvp_imp::PlayerEnterWorld()
{	
	gplayer_imp::PlayerEnterWorld();

	EnableFreePVP(true);
	_runner->player_enable_free_pvp(true);
}

void 
gplayer_pvp_imp::PlayerEnterServer(int source_tag)
{
	gplayer_imp::PlayerEnterServer(source_tag);

	EnableFreePVP(true);
	_runner->player_enable_free_pvp(true);
}

void 
gplayer_pvp_imp::PlayerLeaveServer()
{
	gplayer_imp::PlayerLeaveServer();
	EnableFreePVP(false);
	_runner->player_enable_free_pvp(false);
}

gactive_imp::attack_judge 
gplayer_pvp_imp::GetPetAttackHook()
{
	return gplayer_pvp_imp::__GetPetAttackHook;
	
}

gactive_imp::enchant_judge 
gplayer_pvp_imp::GetPetEnchantHook()
{
	return gplayer_pvp_imp::__GetPetEnchantHook;
}

gactive_imp::attack_fill 
gplayer_pvp_imp::GetPetAttackFill()
{
	return __GetPetAttackFill;
}

gactive_imp::enchant_fill 
gplayer_pvp_imp::GetPetEnchantFill()
{
	return __GetPetEnchantFill;
}

template <typename MESSAGE,typename EFFECT_MSG>
inline static  bool PetTestHarmfulEffect(gactive_imp * __this, const MESSAGE & msg, EFFECT_MSG & emsg)
{
	emsg.is_invader =  false;
	XID attacker = emsg.ainfo.attacker;
	if(attacker.IsPlayerClass())
	{
		if(!emsg.force_attack) return false;

		if(__this->OI_IsInTeam() && __this->OI_IsMember(attacker))
		{
			return 0;
		}

		int invader_state = __this->OI_GetInvaderState();
		if(invader_state == gactive_imp::INVADER_LVL_0 && (emsg.force_attack & C2S::FORCE_ATTACK_NO_WHITE))
		{
		        return false;
		}

		int idmafia = emsg.ainfo.mafia_id;
		if(idmafia)
		{
			if(emsg.force_attack&C2S::FORCE_ATTACK_NO_MAFIA 
					&& idmafia == __this->OI_GetMafiaID()) return false;	
			if(emsg.force_attack&C2S::FORCE_ATTACK_NO_MAFIA_ALLIANCE 
					&& __this->OI_IsFactionAlliance(idmafia)) return false;	
		}

		if(emsg.ainfo.force_id && emsg.ainfo.force_id == __this->OI_GetForceID()) return false;
		
		emsg.target_faction = 0xFFFFFFFF;
	}
	return true;
}

bool 
gplayer_pvp_imp::__GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg)
{
	if( (!(amsg.attacker_mode & attack_msg::PVP_FREE)) || (amsg.attacker_mode & attack_msg::PVP_DUEL))
	{
		return gplayer_imp::__GetPetAttackHook(__this,msg,amsg);
	}
	return PetTestHarmfulEffect(__this,msg,amsg);
				
}

bool 
gplayer_pvp_imp::__GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg)
{
	if( (!(emsg.attacker_mode & attack_msg::PVP_FREE)) || (emsg.attacker_mode & attack_msg::PVP_DUEL))
	{
		return gplayer_imp::__GetPetEnchantHook(__this, msg,emsg);
	}

	if(!emsg.helpful)
	{
		return PetTestHarmfulEffect(__this,msg,emsg);
	}
	emsg.is_invader = false;
	return true;
}

void 
gplayer_pvp_imp::__GetPetAttackFill(gactive_imp * __this, attack_msg & attack)
{
	gplayer_imp::__GetPetAttackFill(__this,attack);
	attack.force_attack |= C2S::FORCE_ATTACK;
	attack.attacker_mode = attack_msg::PVP_FREE;
}

void 
gplayer_pvp_imp::__GetPetEnchantFill(gactive_imp * __this, enchant_msg & enchant)
{
	gplayer_imp::__GetPetEnchantFill(__this,enchant);
	enchant.force_attack |= C2S::FORCE_ATTACK;
	enchant.attacker_mode = attack_msg::PVP_FREE;
}

