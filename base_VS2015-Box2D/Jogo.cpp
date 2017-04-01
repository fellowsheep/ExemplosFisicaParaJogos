#include "Jogo.h"

Jogo::Jogo()
{
}

Jogo::~Jogo()
{
}

void Jogo::inicializar()
{
	uniInicializar(800, 600, false);

	//Carregando as sprites
	gRecursos.carregarSpriteSheet("box", "imagens/teste.png");
	gRecursos.carregarSpriteSheet("circle", "imagens/bola.png");

	//Setando as spritesheets
	boxSpr.setSpriteSheet("box");
	circleSpr.setSpriteSheet("circle");

	//Inicializando o "mundo" da simula��o F�sica
	// Define o vetor da gravidade.
	b2Vec2 gravity(0.0f, 9.8f);
	// Inicializa a biblioteca Box2D
	world = new b2World(gravity);
	// Define os par�metroa para a simula��o
	// Quanto maior, mais preciso, por�m, mais lento
	velocityIterations = 6;
	positionIterations = 2;
	timeStep = 1.0f / 60.0f; //60 Hz

	//cria um corpo r�gido em forma de caixa, que ser� associado a boxSpr
	box = CriaCaixa(gJanela.getLargura()/2, 100);


	//cria paredes
	//Cria o ch�o
	b2BodyDef bd;
	chao = world->CreateBody(&bd);
	//Forma do ch�o: edge
	b2EdgeShape shape;
	shape.Set(b2Vec2(0, 600/SCALE), b2Vec2(800 / SCALE,600 / SCALE));
	chao->CreateFixture(&shape, 0.0);

	//Cria o teto
	teto = world->CreateBody(&bd);
	shape.Set(b2Vec2(0, 0), b2Vec2(800 / SCALE, 0));
	teto->CreateFixture(&shape, 0.0);

	//Cria a parede esquerda
	paredeEsquerda = world->CreateBody(&bd);
	shape.Set(b2Vec2(0, 0), b2Vec2(0, 600 / SCALE));
	paredeEsquerda->CreateFixture(&shape, 0.0);

	//Cria a parede direita
	paredeDireita = world->CreateBody(&bd);
	shape.Set(b2Vec2(800 / SCALE, 0), b2Vec2(800 / SCALE, 600 / SCALE));
	paredeDireita->CreateFixture(&shape, 0.0);
}

void Jogo::finalizar()
{
	//	O resto da finaliza��o vem aqui (provavelmente, em ordem inversa a inicializa��o)!
	//	...

	uniFinalizar();
}

void Jogo::executar()
{
	while(!gTeclado.soltou[TECLA_ESC] && !gEventos.sair)
	{
		uniIniciarFrame();

		world->Step(timeStep, velocityIterations, positionIterations);
		world->ClearForces();

		b2Body *b;
		for (b = world->GetBodyList(); b; b = b->GetNext())
		{
			DrawBody(b);
		}

		if (gTeclado.pressionou[TECLA_F]) //aplica for�a
		{
			b2Vec2 vetorForca;
			vetorForca = CalculaComponentesDoVetor(10000, 45);
			box->ApplyForceToCenter(vetorForca, true);
		}

		if (gMouse.segurando[BOTAO_MOUSE_ESQ])
		{
			CriaCaixa(gMouse.x, gMouse.y);
		}
		if (gMouse.segurando[BOTAO_MOUSE_DIR])
		{
			CriaCirculo(gMouse.x, gMouse.y);
		}

		uniTerminarFrame();
	}
}

b2Body * Jogo::CriaCaixa(float posX, float posY)
{
	b2Body *novoObjeto;

	//Primeiro, criamos a defini��o do corpo
	b2BodyDef bodyDef;
	bodyDef.position.Set(posX/ SCALE, posY/ SCALE);
	bodyDef.type = b2_dynamicBody;

	//Estamos usando uma forma de poligono, que pode ter at� 8 v�rtices
	b2PolygonShape forma;

	//Cada "metro" vai ter 30px 
	forma.SetAsBox((32.0/2.0)/SCALE, (32.0 / 2.0) / SCALE);

	//Depois, criamos uma fixture que vai conter a forma do corpo
	b2FixtureDef fix;
	fix.shape = &forma;
	//Setamos outras propriedades da fixture
	fix.density = 10.0;
	fix.friction = 0.5;
	fix.restitution = 0.5;

	//Por fim, criamos o corpo...
	novoObjeto = world->CreateBody(&bodyDef);
	//... e criamos a fixture do corpo 	
	novoObjeto->CreateFixture(&fix);

	return novoObjeto;
}

b2Body * Jogo::CriaCirculo(float posX, float posY)
{
	b2Body *novoObjeto;

	//Primeiro, criamos a defini��o do corpo
	b2BodyDef bodyDef;
	bodyDef.position.Set(posX / SCALE, posY / SCALE);
	bodyDef.type = b2_dynamicBody;

	//Estamos usando uma forma de poligono, que pode ter at� 8 v�rtices
	b2CircleShape forma;
	//Cada "metro" vai ter 30px 
	forma.m_radius = (32.0 / 2.0) / SCALE;

	//Depois, criamos uma fixture que vai conter a forma do corpo
	b2FixtureDef fix;
	fix.shape = &forma;
	//Setamos outras propriedades da fixture
	fix.density = 10.0;
	fix.friction = 0.5;
	fix.restitution = 0.5;

	//Por fim, criamos o corpo...
	novoObjeto = world->CreateBody(&bodyDef);
	//... e criamos a fixture do corpo 	
	novoObjeto->CreateFixture(&fix);

	return novoObjeto;
}

//Para chamar a rotina de desenho das fixtures de um corpo
void Jogo::DrawBody(b2Body *b)
{
	//desenha s� a primeira fixture. Pra desenhar todas, tem que percorrer a lista
	DrawFixture(b->GetFixtureList());
}

// Fun��o que chama as rotinas de desenho para cada fixture de um corpo r�gido, de acordo com o tipo da forma de colis�o dela e
void Jogo::DrawFixture(b2Fixture* fixture)
{

	const b2Transform& xf = fixture->GetBody()->GetTransform();

	switch (fixture->GetType())
	{
	case b2Shape::e_circle:
	{
		b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

		b2Vec2 center = xf.p;
		float32 radius = circle->m_radius;
		
		float angle = xf.q.GetAngle();
		//desenha a sprite
		circleSpr.desenhar(SCALE* center.x, SCALE*center.y, angle * 180 / b2_pi);
	}
	break;

	case b2Shape::e_polygon:
	{
		//desenha a sprite
		b2PolygonShape *poly = (b2PolygonShape*)fixture->GetShape();
		b2Vec2 center = xf.p;
		float angle = xf.q.GetAngle();
		boxSpr.desenhar(SCALE* center.x, SCALE*center.y,angle*180/b2_pi);
		
	}

	break;
	case b2Shape::e_edge:
	{
		b2EdgeShape* edge = (b2EdgeShape*)fixture->GetShape();
		int32 vertexCount;

		b2Vec2 vertices[b2_maxPolygonVertices];
		int i = 0;

		if (edge->m_hasVertex0)
		{
			vertices[i] = b2Mul(xf, edge->m_vertex0);
			i++;
		}
		vertices[i] = b2Mul(xf, edge->m_vertex1); i++;
		vertices[i] = b2Mul(xf, edge->m_vertex2); i++;
		if (edge->m_hasVertex3)
		{
			vertices[i] = b2Mul(xf, edge->m_vertex3);
			i++;
		}

		vertexCount = i;
		
		//renderer.DrawPolygon(vertices, vertexCount, color);
	}

	break;

	}

}

//Rotina que calcula as componentes vx e vy do vetor formado pelo comprimento (magnitude) fornecido e o �ngulo
b2Vec2 Jogo::CalculaComponentesDoVetor(float magnitude, float angulo)
{
	float angulorad = -angulo*b2_pi / 180;
	float vx = magnitude*cos(angulorad);
	float vy = magnitude*sin(angulorad);
	b2Vec2 vec(vx, vy);
	return vec;
}