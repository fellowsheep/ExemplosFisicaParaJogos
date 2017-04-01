#pragma once
#include "libUnicornio.h"
#include "Box2D/Box2D.h"

const float SCALE = 30.0; //cada metro equivalerá a 30 pixels

class Jogo
{
protected:
	b2World *world;
	b2Body *box, *chao, *paredeDireita, *paredeEsquerda, *teto;
	Sprite boxSpr;
	Sprite circleSpr;

	//parâmetros da simulação
	float32 timeStep;
	int32 velocityIterations;
	int32 positionIterations;

public:
	Jogo();
	~Jogo();

	void inicializar();
	void finalizar();

	void executar();
	b2Body* CriaCaixa(float posX, float posY);
	b2Body* CriaCirculo(float posX, float posY);
	void DrawBody(b2Body *b);
	void DrawFixture(b2Fixture* fixture);

	//isso vai para outra classe depois
	b2Vec2 CalculaComponentesDoVetor(float magnitude, float angulo);
};

