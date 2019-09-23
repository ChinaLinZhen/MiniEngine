#include "MiniEngine.h"

namespace engine {

	Input::Input(const char* name/*, const std::vector<Node* >& inputs*/)
	{
		_name = name;
	}

	void Input::forward(const Eigen::MatrixXf& value) {
		_value = value;
	}

	void Input::backward() {
		for (auto node : _outputs) {
			_gradients[this] = node->getGradient(this);
		}
	}

	Linear::Linear(Node* nodes, Node* weights, Node* bias)
		:Node(std::vector<Node*>({nodes,weights,bias}))
		,_nodes(nodes)
		,_weights(weights)
		,_bias(bias)
	{
		_name = "Linear";
	}

	void Linear::forward(const Eigen::MatrixXf& )
	{
		_value = _nodes->getValue() * _weights->getValue() + _bias->getValue();
	}

	void Linear::backward()
	{
		for (auto node : _outputs) {
			auto grad = node->getGradient(this);
			_gradients[_weights] = _nodes->getValue().transpose() * grad;
			//�������,��Ϊһ��
			_gradients[_bias] = grad.colwise().sum();
			_gradients[_nodes] = grad * _weights->getValue().transpose();
		}
	}

	Sigmoid::Sigmoid(Node* node)
		:Node(std::vector<Node*>({node}))
		,_node(node)
	{
		_name = "Sigmoid";
	}

	void Sigmoid::forward(const Eigen::MatrixXf& )
	{
		_value = _impl(_node->getValue());
	}

	void Sigmoid::backward()
	{
		auto y = _value;
		_partial = y-y*y;

		for (auto node : _outputs) {
			auto grad = node->getGradient(this);
			_gradients[_node] = grad * _partial;
		}
	}

	Eigen::MatrixXf Sigmoid::_impl(const Eigen::MatrixXf& x)
	{
		return (x.array().exp() + 1).inverse();
	}

	MSE::MSE(Node* y, Node* y_hat)
		:Node(std::vector<Node*>({y,y_hat}))
		,_y(y),_y_hat(y_hat)
	{
		_name = "MSE";
	}

	void MSE::forward(const Eigen::MatrixXf& value)
	{
		_diff = _y->getValue() - _y_hat->getValue();
		auto v = Eigen::MatrixXf(1, 1);
		v<<(_diff * _diff).mean();
		_value = v;
	}

	void MSE::backward()
	{
		auto r = _y_hat->getValue().rows();
		_gradients[_y] = _diff * (2 / r);
		_gradients[_y_hat] = _diff * (-2 / r);
	}


	std::vector<Node*> topological_sort(Node* input_nodes)
	{
		//���ݴ�������ݳ�ʼ��ͼ�ṹ
		Node* pNode = nullptr;
		//pair��һ��Ϊ����,�ڶ���Ϊ���
		std::map < Node*, std::pair<std::vector<Node*>, std::vector<Node*> > > g;
		//����������Χ�ڵ�
		std::list<Node*> vNodes;
		vNodes.emplace_back(input_nodes);
		//�Ѿ��������Ľڵ�
		std::set<Node*> sVisited;
		while (vNodes.size() && (pNode = vNodes.front())) {
			const auto& outputs = pNode->getOutputs();
			//��ȱ���,�ȱ�������ڵ�,�ٱ�������ڵ�
			for (auto item: outputs)
			{
				g[pNode].second.emplace_back(item);	//���itemΪpnode������ڵ�
				g[item].first.emplace_back(pNode);	//���pnodeΪitem������ڵ�
				if(sVisited.find(item)==sVisited.end()) vNodes.emplace_back(item);	//��û�з��ʹ��Ľڵ���ӵ������ʶ�����
			}
			const auto& inputs = pNode->getInputs();
			for (auto item: inputs)
			{
				g[pNode].first.emplace_back(item);	//���itemΪpnode������ڵ�
				g[item].first.emplace_back(pNode);	//���pnodeΪitem������ڵ�
				if (sVisited.find(item) == sVisited.end()) vNodes.emplace_back(item);
			}
			std::cout << pNode->name() << std::endl;
			sVisited.emplace(pNode);
			vNodes.pop_front();
		}

		//����ͼ�ṹ������������
		std::vector<Node*> vSorted;
		while (pNode->getOutputs().size()) {

		}
		return vSorted;
	}

}